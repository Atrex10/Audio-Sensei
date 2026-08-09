import audio_control
import json
import sys
import logging
from pathlib import Path

audio = audio_control.AudioController()
playback = audio_control.PlaybackController()

def loadMapping(mappingPath: str) -> dict:
    path = Path(mappingPath)
    if not path.is_file():
        logging.critical(f"Mapping error: {mappingPath} does not exist")
        sys.exit(1)

    with open(path, "r") as f:
        data = json.load(f)
        logging.debug(f"Loaded inputs mapping:\n{data}")
        return data

def executeCommand(mapping: dict, Input: str, inputID: int, _value: int) -> tuple[bool, str]:  # Uppercase I in Input to avoid name shadowing
    # try to get input mapping field
    try:
        input_field: dict = mapping[Input][inputID]

    except KeyError as e:
        logging.warning(f"Received wrong input name, details:\n{e}")
        return False, "ERROR"
    except IndexError as e:
        logging.warning(f"Received input ID out of range, details:\n{e}")
        return False, "ERROR"

    # getting mapping values
    try:
        _command: str = input_field["command"]
    except KeyError:
        logging.warning('No "command" property found in input map, using None')
        _command: str = "None"

    try:
        _multiplier: int = input_field["value_multiplier"]
    except KeyError:
        logging.debug('No "value_multiplier" property found in input map, using 1')
        _multiplier: int = 1

    try:
        _apps: list[str] = input_field["apps"]
    except KeyError:
        logging.debug('No "apps" property found in input map, using empty list')
        _apps: list[str] = []


    # finally evaluating the result and returning
    try:
        result: str = eval(_command)
        logging.debug(f"Evaluated: {_command} to be: {result}")
    except Exception as e:
        logging.error(f"Failed to evaluate: {_command}:\n{e}")
        return False, "ERROR"

    return True, result


if __name__ == "__main__":
    logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s"
    )
    executeCommand(loadMapping("/mnt/1t-storage/Files/Wojtek/Projects/Serial-Audio-Controller/code/pc-daemon/mapping.json"), "button", 2, 1)
