import logging
import serial
import json
import tomllib
import sys
from pathlib import Path
import time
import argparse
import os

import command_mapping

# Logger init
# Default logging to stdout, systemd will redirect to journalctl
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# VARIABLES
# SERIAL_PORT: str = "/tmp/ACM0"
# BAUD_RATE: int = 115200

# only for testing
# test_line: str = '{ "type": "command", "id": 2, "status": "OK", "input": "encoder", "inputID": 0, "value": 2 }'


def loadConfig(configPath: str) -> dict:
    path = Path(configPath)
    if not path.is_file():
        logging.critical(f"Config error: {configPath} does not exist")
        sys.exit(1)

    with open(path, "rb") as f:
        data = tomllib.load(f)
        logging.debug(f"Loaded config:\n{data}")
        return data


def sendResponse(ser: serial.Serial, status: str, cmd_response: str, msg_id: str) -> None:
    response: dict = {
        "type": "ack",
        "id": msg_id,
        "status": "OK",  # OK or ERROR
        "command": "confExec",
        "value": cmd_response
    }

    while True:
        try:
            payload = json.dumps(response) + "\n"
            ser.write(payload.encode("utf-8"))
            ser.flush()
            logging.info(f"ACK sent [ID: {msg_id}] status: {status}")
            break
        except Exception as e:
            logging.error(f"Sending ACK failed: {e}. Retrying in 1s...")
            time.sleep(1)
            continue

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

    while True:
        # load the mapping json file
        mapping = command_mapping.loadMapping(args.mapping)
        config = loadConfig(args.config)

        try:
            config["serial"]["port"]
            config["serial"]["baudrate"]
        except KeyError:
            logging.critical("Invalid configuration file, some fields may be missing")
            sys.exit(1)

        try:
            logging.debug(f"Connecting to {config["serial"]["port"]}...")
            with serial.Serial(config["serial"]["port"], config["serial"]["baudrate"], timeout=1) as ser:
                logging.info(f"Connected to {config["serial"]["port"]}")

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
                        cmd_input = data.get("input")
                        cmd_inputID = data.get("inputID")
                        cmd_value = data.get("value")
                        msg_id: str = str(data.get("id", "0"))

                        logging.debug(f"Decoded JSON command:\ninput: {cmd_input}\ninput ID: {cmd_inputID}\nvalue: {cmd_value}\nmessage id: {msg_id}")

                        # Safely convert cmd_value & cmd_inputID to int when possible
                        try:
                            cmd_value_int = int(cmd_value) if cmd_value is not None else 0
                        except (ValueError, TypeError):
                            logging.warning(f"Invalid input value: {cmd_value}. Using 0")
                            cmd_value_int = 0
                        try:
                            cmd_inputID_int = int(cmd_inputID) if cmd_inputID is not None else 0
                        except (ValueError, TypeError):
                            logging.warning(f"Invalid input ID: {cmd_inputID}. Using 0")
                            cmd_inputID_int = 0

                        # executing the command
                        success, response = command_mapping.executeCommand(mapping, str(cmd_input), cmd_inputID_int, cmd_value_int)

                        status = "OK" if success else "ERROR"
                        sendResponse(ser, status, response, msg_id)

                    else:
                        logging.warning(f'No "command" field in packet: {data}')


        except serial.SerialException as e:
            logging.debug(f"Failed serial port connection with {config["serial"]["port"]}: {e}. Reconnecting in 1s...")
            time.sleep(1)

        except Exception as e:
            logging.critical(f"Unexpected error in main loop: {e}. Rerunning the loop in 5s...")
            time.sleep(5)


if __name__ == "__main__":
    main()
