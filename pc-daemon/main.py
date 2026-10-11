import logging
import serial
import glob
import json
import tomllib
import sys
import time
import argparse
import os
import subprocess

# Logger init
# Default logging to stdout, systemd will redirect to journalctl
logging.basicConfig(
    level=logging.WARNING,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

HANDLER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "handler.py")

# VARIABLES
# SERIAL_PORT: str = "/tmp/ACM0"
# BAUD_RATE: int = 115200

# only for testing
# test_line: str = '{ "type": "command", "id": 2, "status": "OK", "input": "encoder", "inputID": 0, "value": 2 }'

def find_controller_port(product_name: str = "AudioSensei_Controller"):
    matches = glob.glob(f"/dev/serial/by-id/*{product_name}*")
    return matches[0] if matches else None


def loadConfig(configPath: str) -> dict:
    if not os.path.isfile(configPath):
        logging.critical(f"Config error: {configPath} does not exist")
        sys.exit(1)

    with open(configPath, "rb") as f:
        data = tomllib.load(f)
        logging.debug(f"Loaded config:\n{data}")
        return data


def sendResponse(ser: serial.Serial, status: str, cmd_response: str, msg_id: int, max_retries: int=3) -> bool:
    response: dict = {
        "type": "ack",
        "id": msg_id,
        "status": status,  # OK or ERROR
        "command": "confExec",
        "value": cmd_response
    }

    payload = (json.dumps(response) + "\n").encode("utf-8")

    ser.write_timeout = 0  # fire-and-forget, non blocking serial write

    for attempt in range(1, max_retries + 1):
        try:
            if not ser.is_open:
                raise serial.SerialException("Serial port is closed.")

            ser.write(payload)
            # ser.flush()
            logging.info(f"ACK sent [ID: {msg_id}] status: {status}")
            return True

        except (serial.SerialException, Exception) as e:
            logging.error(
                f"Sending ACK failed (Attempt {attempt}/{max_retries}): {e}"
            )
            if attempt < max_retries:
                time.sleep(1)

    logging.critical(
        f"Failed to send ACK [ID: {msg_id}] after {max_retries} attempts."
    )
    return False

def main() -> None:
    parser = argparse.ArgumentParser(description="Serial-Audio-Controller daemon")
    parser.add_argument(
        "-c", "--config",
        default=f"{os.path.expanduser('~')}/.config/Audio-Sensei-daemon/config.toml",
        help="Path to configuration file"
    )

    parser.add_argument(
        "-m", "--mapping",
        default=f"{os.path.expanduser('~')}/.config/Audio-Sensei-daemon/mapping.json",
        help="Path to inputs mapping file"
    )

    args = parser.parse_args()

    # load the config toml file
    config = loadConfig(args.config)

    try:
        config["serial"]["product"]
        config["serial"]["baudrate"]
    except KeyError:
        logging.critical("Invalid configuration file, some fields may be missing")
        sys.exit(1)

    while True:
        try:
            logging.debug(f"Searching for serial port for {config["serial"]["product"]}...")
            port = find_controller_port(config["serial"]["product"])
            if not port:
                logging.debug(f"Serial port for {config["serial"]["product"]} not found, retrying in 10s...")
                time.sleep(10)
                continue

            logging.debug(f"Connecting to {port}...")
            with serial.Serial(port, config["serial"]["baudrate"], timeout=None) as ser:
                logging.info(f"Connected to {port}")

                while True:
                    raw_line: str = ser.readline().decode("utf-8", errors="ignore").strip()
                    # raw_line: str = test_line  # for testing

                    logging.debug(f"Received line: {raw_line}")

                    if not raw_line:
                        continue

                    try:
                        data = json.loads(raw_line)
                    except json.JSONDecodeError:
                        logging.warning(f"JSON parsing error: {raw_line}")
                        continue

                    if isinstance(data, dict) and "input" in data and "inputID" in data:
                        try:
                            proc = subprocess.run(
                                [sys.executable, HANDLER, args.mapping],
                                input=raw_line, text=True,
                                stdout=subprocess.PIPE,  # stderr is inherited -> goes to journald
                                timeout=10
                                )

                            result = json.loads(proc.stdout)
                            status = "OK" if result["success"] else "ERROR"
                            response = result["response"]

                        except Exception as e:
                            logging.error(f"Handler failed: {e}")
                            status, response = "ERROR", "ERROR"

                        sendResponse(ser, status, response, data.get("id", "0"))

                    else:
                        logging.warning(f'No "command" field in packet: {data}')


        except serial.SerialException as e:
            logging.error(f"Failed serial port connection with {config["serial"]["port"]}: {e}. Reconnecting in 1s...")
            time.sleep(1)

        except Exception as e:
            logging.critical(f"Unexpected error in main loop: {e}. Rerunning the loop in 5s...")
            time.sleep(5)


if __name__ == "__main__":
    main()
