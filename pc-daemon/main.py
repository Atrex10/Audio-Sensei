import logging

# Logger init
# Default logging to stdout, systemd will redirect to journalctl
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# VARIABLES
SERIAL_PORT: str = "/dev/ttyACM0"
BAUD_RATE: int = 115200


def main() -> None:
    pass


if __name__ == "__main__":
    main()
