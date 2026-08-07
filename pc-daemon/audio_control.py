import pulsectl
import subprocess
import logging

METADATA_KEYS: list[str] = ["title", "artist", "album"]

class PlaybackController:
    def pausePlay(self) -> None:
        subprocess.run(["playerctl", "play-pause"])

    def stop(self) -> None:
        subprocess.run(["playerctl", "stop"])

    def pause(self) -> None:
        subprocess.run(["playerctl", "pause"])

    def play(self) -> None:
        subprocess.run(["playerctl", "play"])

    def nextTrack(self) -> None:
        subprocess.run(["playerctl", "next"])

    def prevTrack(self) -> None:
        subprocess.run(["playerctl", "previous"])

    def setPosition(self, position: int=0) -> None:
        subprocess.run(["playerctl", "position", str(position)])

    def changePosition(self, change: int=0) -> None:
        change_str: str = str(abs(change)) + '+' if change > 0 else str(abs(change)) + '-'
        subprocess.run(["playerctl", "position", change_str])

    def setVolume(self, volume:float=0) -> None:
        volume = round(float(volume/100), 2)
        subprocess.run(["playerctl", "volume", str(volume)])
    
    def changeVolume(self, change: float=0) -> None:
        change = round(float(change/100), 2)
        change_str: str = str(abs(change)) + '+' if change > 0 else str(abs(change)) + '-'
        subprocess.run(["playerctl", "volume", change_str])

    def setShuffle(self, state: bool) -> None:
        state_str: str = "On" if state else "Off"
        subprocess.run(["playerctl", "shuffle", state_str])

    def toggleShuffle(self) -> None:
        subprocess.run(["playerctl", "shuffle", "toggle"])

    def setLoop(self, state: int) -> None:
        match (state):
            case 1:
                state_str: str = "Track"
            case 2:
                state_str: str = "Playlist"
            case _:
                state_str: str = "None"

        subprocess.run(["playerctl", "loop", state_str])

    def toggleLoop(self) -> None:
        result = subprocess.run(["playerctl", "loop"], capture_output=True, text=True)

        logging.debug(f"playerctl loop: {result.stdout}")

        # convert text to index
        match (result.stdout.strip()):
            case "Track":
                current: int = 1
            case "Playlist":
                current: int = 2
            case _:
                current: int = 0

        current += 1

        self.setLoop(current)

    def getMetadata(self) -> dict:
        metadata: dict = {}
        for key in METADATA_KEYS:
            result = subprocess.run(["playerctl", "metadata", key], capture_output=True, text=True)
            logging.debug(f"playerctl metadata {key}: {result.stdout.strip()}")
            print(result.stdout.strip())
            if result.stdout.strip():
                metadata[key] = result.stdout.strip()
            else:
                metadata[key] = "---"

        result = subprocess.run(["playerctl", "status"], capture_output=True, text=True)
        logging.debug(f"playerctl status: {result.stdout.strip()}")
        if result.stdout.strip() == "Playing":
            metadata["playing"] = True
        else:
            metadata["playing"] = False

        return metadata


