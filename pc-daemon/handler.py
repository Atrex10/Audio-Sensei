import sys
import json
import logging

def to_int(v, label):
    try:
        return int(v) if v is not None else 0
    except (ValueError, TypeError):
        logging.warning(f"Invalid {label}: {v}. Using 0")
        return 0


def main() -> None:
    out = sys.stdout
    sys.stdout = sys.stderr  # stray print()s must not currupt the result

    # config the logger
    logging.basicConfig(
        level=logging.DEBUG,
        format="%(asctime)s [%(levelname)s] %(message)s"
    )

    import command_mapping

    mapping = command_mapping.loadMapping(sys.argv[1])
    data = json.loads(sys.stdin.readline())

    success, response = command_mapping.executeCommand(
        mapping,
        str(data["input"]),
        to_int(data["inputID"], "input ID"),
        to_int(data.get("value"), "input value"),
    )
    out.write(json.dumps({"success": success, "response": response}))

if __name__ == "__main__":
    main()

