import subprocess
import json
import sys
import logging
from pathlib import Path


def loadMapping(configPath: str) -> dict:
    path = Path(configPath)
    if not path.is_file():
        logging.error(f"Mapping error: {configPath} does not exist")
        sys.exit(1)

    with open(path, "r") as f:
        logging.debug(f"Loaded inputs mapping:\n{json.loads(f.read())}")
        return json.loads(f.read())


def executeCommand(mapping: dict, Input: str, inputID: int, value: int) -> tuple[bool, str]:  # Uppercase I in Input to avoid name shadowing
    match Input:
        case "volume":
            pass
        case "playback":
            pass


    return False, "Failed"
