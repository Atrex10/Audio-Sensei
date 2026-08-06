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
SERIAL_PORT: str = "/dev/ttyACM0"
BAUD_RATE: int = 115200

# only for testing
test_line: str = '{ "type": "command", "id": 2, "status": "OK", "command": "volume1", "value": 20 }'

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

                        # TODO: write command execution and ack response

        except serial.SerialException as e:
            logging.error(f"Lost serial port connection with {SERIAL_PORT}: {e}. Reconnecting in 5s...")
            time.sleep(5)

        except Exception as e:
            logging.critical(f"Unexpected error in main loop: {e}. Rerunning the loop in 5s...")
            time.sleep(5)



if __name__ == "__main__":
    main()
