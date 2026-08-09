#!/bin/bash

# removing user
systemctl --user disable Audio-Sensei-daemon.service
systemctl --user stop Audio-Sensei-daemon.service

# removing files
rm -rf /home/$USER/.local/share/Audio-Sensei-daemon

rm -rf /home/$USER/.config/Audio-Sensei-daemon

rm -f /home/$USER/.config/systemd/user/Audio-Sensei-daemon.service