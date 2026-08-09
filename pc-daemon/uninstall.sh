#!/bin/bash

# removing user
systemctl --user disable SAC-daemon.service
systemctl --user stop SAC-daemon.service

# removing files
rm -rf /home/$USER/.local/share/SAC-daemon

rm -rf /home/$USER/.config/SAC-daemon

rm -f /home/$USER/.config/systemd/user/SAC-daemon.service