import pulsectl
import subprocess
import logging
import time

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

class AudioController:
    def __init__(self) -> None:
        while True:
            try:
                self.pulse = pulsectl.Pulse("Audio-Sensei-pulse-daemon")
                break
            except Exception as e:
                logging.critical(f"Failed to connect to audio server: {e}. Retrying in 5s...")
                time.sleep(5)


    def getSinkInputsFromList(self, apps_names: list[str]) -> list:
        sink_inputs = self.pulse.sink_input_list()

        inputs: list = []

        for sink_input in sink_inputs:
            input_name = sink_input.proplist.get("application.name", "Unknown application")
            logging.debug(f"Detected app: {input_name}")

            if input_name in apps_names:
                inputs.append(sink_input)

        logging.debug(f"Found sink inputs: {inputs} for names: {apps_names}")
        return inputs

    def getDefaultOutputSink(self):
        sinks = self.pulse.sink_list()
    
        # find default device
        default_sink_name = self.pulse.server_info().default_sink_name # type: ignore
        default_sink = next(s for s in sinks if s.name == default_sink_name)

        return default_sink

    # APPS
    # volume stuff
    def setAppsVolume(self, apps_names: list[str], volume: float) -> None:
        """
        Set volume in specified apps (at once)
        App name is an application.name property of the sink input
        :param apps_names: a list of app names the function will affect
        :param volume: volume in range 0-100 (percent)
        """

        # user error handling
        volume = volume / 100  # convert from percent to float
        volume = max(volume, 0)
        volume = min(volume, 1)

        sink_inputs = self.getSinkInputsFromList(apps_names)

        if not sink_inputs:
            return
        
        for sink_input in sink_inputs:
            self.pulse.volume_set_all_chans(sink_input, volume)

    def getAppVolume(self, app_name: str) -> int:
        """
        Read app volume
        App name is an application.name property of the sink input
        :param app_name: name of the app to read from
        :return: volume in range 0-100 (percent)
        """

        sink_input = self.getSinkInputsFromList([app_name])

        if sink_input:
            return int(self.pulse.volume_get_all_chans(sink_input[0]) * 100)
        else:
            logging.debug(f"getAppVolume: Did not find active sink input for {app_name}, returning 0")
            return 0

    def changeAppsVolume(self, apps_names: list[str], volume: float) -> None:
        """
        Change volume in specified apps by specified amount (at once)
        App name is an application.name property of the sink input
        :param apps_names: a list of app names the function will affect
        :param volume: volume offset/change, in percent
        """

        sink_inputs = self.getSinkInputsFromList(apps_names)

        volume = volume / 100  # convert from percent to float

        if not sink_inputs:
            return

        for sink_input in sink_inputs:
            input_volume = self.pulse.volume_get_all_chans(sink_input)
            # logging.debug(f"Current volume for {sink_input}: {input_volume}")
            input_volume += volume  # calculate new app volume

            # limiting range to prevent soft boost
            input_volume = max(input_volume, 0)
            input_volume = min(input_volume, 1)

            self.pulse.volume_set_all_chans(sink_input, input_volume)

    # mute stuff
    def setAppsMute(self, apps_names: list[str], mute: bool=True) -> None:
        """
        Set apps mute state to specified (at once)
        App name is an application.name property of the sink input
        :param apps_names: a list of app names the function will affect
        :param mute: new mute state of the apps
        """

        sink_inputs = self.getSinkInputsFromList(apps_names)

        if not sink_inputs:
            return

        for sink_input in sink_inputs:
            self.pulse.mute(sink_input, mute=mute)

    def toggleAppsMute(self, apps_names: list[str]) -> None:
        """
        Toggle mute states in specified appss (at once)
        Notice, that this function will toggle all the states independently,
        and might not set all apps to the same state
        App name is an application.name property of the sink input
        :param apps_names: a list of app names the function will affect
        """

        sink_inputs = self.getSinkInputsFromList(apps_names)

        if not sink_inputs:
            return

        for sink_input in sink_inputs:
            self.pulse.mute(sink_input, mute=not sink_input.mute)

    def getAppMute(self, app_name: str) -> bool:
        """
        Read app mute state
        App name is an application.name property of the sink input
        :param apps_name: name of the app to read from
        :return: read mute state in bool type
        """

        sink_input = self.getSinkInputsFromList([app_name])
        
        if sink_input:
            return sink_input[0].mute
        else:
            logging.debug(f"getAppMute: Did not find active sink input for {app_name}, returning False")
            return False

    # DEVICES
    # outputs
    # for now only volume on default (currently active) device (master volume)
    # control is implemented, as there is no need for anything else

    # volume stuff
    def setMasterVolume(self, volume: float) -> None:
        """
        Set master volume of the system
        This will set the volume of the default (currently active)
        output device, effectively master volume
        :param volume: volume in range 0-100 (percent)
        """

        sink = self.getDefaultOutputSink()

        # user error handling
        volume = volume / 100  # convert from percent to float
        volume = max(volume, 0)
        volume = min(volume, 1)

        if sink:
            self.pulse.volume_set_all_chans(sink, volume)

    def getMasterVolume(self) -> int:
        """
        Get master volume of the system
        This will read the volume of the default (currently active)
        output device, effectively master volume
        :return: volume in range 0-100 (percent)
        """
        
        sink = self.getDefaultOutputSink()

        if sink:
            return int(self.pulse.volume_get_all_chans(sink) * 100)
        else:
            logging.warning("getMasterVolume: No default output device found, returning 0")
            return 0

    def changeMasterVolume(self, volume: float) -> None:
        """
        Change system master volume by specified amount
        This will set the volume of the default (currently active)
        output device, effectively master volume
        :param volume: volume offset/change in percent
        """
        sink = self.getDefaultOutputSink()

        volume = volume / 100

        if sink:
            current_volume = self.getMasterVolume() / 100
            current_volume += volume  # calculate new master volume

            # limiting range to prevent soft boost
            current_volume = max(current_volume, 0)
            current_volume = min(current_volume, 1)

            self.pulse.volume_set_all_chans(sink, current_volume)


    # mute stuff
    def setMasterMute(self, mute: bool=True) -> None:
        """
        Set system master volume mute state to specified
        This will set the mute state of the default (currently active)
        output device, effectively master volume
        :param mute: new mute state of the master volume
        """

        sink = self.getDefaultOutputSink()

        if sink:
            self.pulse.mute(sink, mute=mute)

    def toggleMasterMute(self) -> None:
        """
        Toggle system master volume mute state
        This will set the mute state of the default (currently active)
        output device, effectively master volume
        """

        sink = self.getDefaultOutputSink()

        if sink:
            self.pulse.mute(sink, mute=not sink.mute)

    def getMasterMute(self) -> bool:
        """
        Read system master volume mute state
        This will read the mute state of the default (currently active)
        output device, effectively master volume
        :return: read mute state in bool type
        """

        sink = self.getDefaultOutputSink()
        
        if sink:
            return sink.mute
        else:
            logging.warning("getMasterMute: No default output device found, returning False")
            return False

    # TODO: add inputs control in the future

if __name__ == "__main__":
    audio = AudioController()
    audio.changeMasterVolume(10)
