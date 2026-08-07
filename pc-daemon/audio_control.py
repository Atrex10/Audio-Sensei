import pulsectl
import subprocess
import logging

METADATA_KEYS: list[str] = ["title", "artist", "album"]

class PlaybackController:
    def pausePlay(self) -> None:
        """Pause or unpause current player"""
        subprocess.run(["playerctl", "play-pause"])

    def stop(self) -> None:
        """Stop current player"""
        subprocess.run(["playerctl", "stop"])

    def pause(self) -> None:
        """Pause current player"""
        subprocess.run(["playerctl", "pause"])

    def play(self) -> None:
        """Unpause current player"""
        subprocess.run(["playerctl", "play"])

    def nextTrack(self) -> None:
        """Skip to next track"""
        subprocess.run(["playerctl", "next"])

    def prevTrack(self) -> None:
        """Go to start/previous track"""
        subprocess.run(["playerctl", "previous"])

    def setPosition(self, position: int=0) -> None:
        """
        Set playback position
        :param position: Position in seconds
        """

        position = max(position, 0)

        subprocess.run(["playerctl", "position", str(position)])

    def changePosition(self, change: int=0) -> None:
        """
        Change (offset) playback position
        :param change: Offset in seconds
        """

        change_str: str = str(abs(change)) + '+' if change >= 0 else str(abs(change)) + '-'
        subprocess.run(["playerctl", "position", change_str])

    def setVolume(self, volume:float=0) -> None:
        """
        Set given volume of current player
        :param volume: Volume in range 0-100 (percent)
        """

        volume = max(volume, 0)
        volume = min(volume, 100)

        volume = round(float(volume/100), 2)

        subprocess.run(["playerctl", "volume", str(volume)])
    
    def changeVolume(self, change: float=0) -> None:
        """
        Change/offset volume of current player
        :param volume: Volume offset in percent
        """

        change = round(float(change/100), 2)
        change_str: str = str(abs(change)) + '+' if change >= 0 else str(abs(change)) + '-'
        subprocess.run(["playerctl", "volume", change_str])

    def setShuffle(self, state: bool) -> None:
        """
        Set current player shuffle state
        :param state: boolean, represents on or off
        """

        state_str: str = "On" if state else "Off"
        subprocess.run(["playerctl", "shuffle", state_str])

    def toggleShuffle(self) -> None:
        """Toggle current player shuffle state"""
        subprocess.run(["playerctl", "shuffle", "toggle"])

    def setLoop(self, state: int) -> None:
        """
        Set current player loop state
        :param state: 0 is none, 1 is track, 2 is context(playlist)
        """

        match (state):
            case 1:
                state_str: str = "Track"
            case 2:
                state_str: str = "Playlist"
            case _:
                state_str: str = "None"

        subprocess.run(["playerctl", "loop", state_str])

    def toggleLoop(self) -> None:
        """Cycle through current player loop states"""
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
        current = current % 3

        self.setLoop(current)

    def getMetadata(self) -> dict:
        """
        Read current player metadata (title, artist, album, status) and return
        :return: dictionary with keys mentioned above and their read values
        """

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


