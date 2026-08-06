import logging
import serial
import json
import time

# Logger init
# Default logging to stdout, systemd will redirect to journalctl
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# VARIABLES
SERIAL_PORT: str = "/tmp/ttyV0"
BAUD_RATE: int = 115200

# only for testing
test_line: str = '{ "type": "command", "id": 2, "status": "OK", "command": "volume1", "value": 20 }'


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
    while True:
        try:
            logging.info(f"Connecting to {SERIAL_PORT}...")
            with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
                logging.info(f"Connected to {SERIAL_PORT}")

                while True:
                    # raw_line: str = ser.readline().decode("utf-8", errors="ignore").strip()
                    raw_line: str = test_line  # for testing

                    if not raw_line:
                        continue

                    try:
                        data = json.loads(raw_line)
                    except json.JSONDecodeError:
                        logging.warning(f"JSON parsing error: {raw_line}")
                        continue

                    if isinstance(data, dict) and "command" in data:
                        cmd_name = data.get("command")
                        cmd_value = data.get("value")
                        msg_id: str = str(data.get("id", "0"))

                        logging.debug(f"Decoded JSON command:\ncommand: {cmd_name}\nvalue: {cmd_value}\nmessage id: {msg_id}")

                        # TODO: write command execution and get status from there
                        # temporary:
                        success: bool = True
                        response: str = "Nawet prosty ten kod"

                        status = "OK" if success else "ERROR"
                        sendResponse(ser, status, response, msg_id)

                    else:
                        logging.warning(f'No "command" field in packet: {data}')


        except serial.SerialException as e:
            logging.error(f"Lost serial port connection with {SERIAL_PORT}: {e}. Reconnecting in 5s...")
            time.sleep(5)

        except Exception as e:
            logging.critical(f"Unexpected error in main loop: {e}. Rerunning the loop in 5s...")
            time.sleep(5)



if __name__ == "__main__":
    main()
