#!/bin/bash

# preparing destination folders
if [ ! -d "/home/$USER/.local" ]; then
    mkdir /home/$USER/.local
fi
if [ ! -d "/home/$USER/.local/share" ]; then
    mkdir /home/$USER/.local/share
fi
if [ ! -d "/home/$USER/.config" ]; then
    mkdir /home/$USER/.config
fi
if [ ! -d "/home/$USER/.config/SAC-daemon" ]; then
    mkdir /home/$USER/.config/SAC-daemon
fi
if [ ! -d "/home/$USER/.config/systemd" ]; then
    mkdir /home/$USER/.config/systemd
fi
if [ ! -d "/home/$USER/.config/systemd/user" ]; then
    mkdir /home/$USER/.config/systemd/user
fi

# copying source code
cp -r ./ /home/$USER/.local/share/SAC-daemon/

# copying mapping file 
cp ./mapping.json /home/$USER/.config/SAC-daemon/mapping.json

# copying systemd service unit
cp ./SAC-daemon.service /home/$USER/.config/systemd/user/SAC-daemon.service

# systemd reload and starting deamon
systemctl --user daemon-reload
systemctl --user enable --now SAC-daemon.service

