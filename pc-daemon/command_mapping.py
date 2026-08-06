import subprocess

def executeCommand(Input: str, inputID: int, value: int) -> tuple[bool, str]:  # Uppercase I in input to avoid name shadowing
    match Input:
        case "volume":
            pass
        case "playback":
            pass


    return False, "Failed"
