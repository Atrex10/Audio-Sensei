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
if [ ! -d "/home/$USER/.config/Audio-Sensei-daemon" ]; then
    mkdir /home/$USER/.config/Audio-Sensei-daemon
fi
if [ ! -d "/home/$USER/.config/systemd" ]; then
    mkdir /home/$USER/.config/systemd
fi
if [ ! -d "/home/$USER/.config/systemd/user" ]; then
    mkdir /home/$USER/.config/systemd/user
fi

# create virtual environment and install dependencies
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
deactivate

# copying source code
cp -r ./ /home/$USER/.local/share/Audio-Sensei-daemon/

# remove the unecesary files from the source code directory
rm /home/$USER/.local/share/Audio-Sensei-daemon/mapping.json
rm /home/$USER/.local/share/Audio-Sensei-daemon/config.toml
rm /home/$USER/.local/share/Audio-Sensei-daemon/install.sh
rm /home/$USER/.local/share/Audio-Sensei-daemon/uninstall.sh

# copying mapping file 
cp ./mapping.json /home/$USER/.config/Audio-Sensei-daemon/mapping.json

# copying config file
cp ./config.toml /home/$USER/.config/Audio-Sensei-daemon/config.toml

# copying config file
cp ./config.toml /home/$USER/.config/Audio-Sensei-daemon/config.toml

# copying systemd service unit
cp ./Audio-Sensei-daemon.service /home/$USER/.config/systemd/user/Audio-Sensei-daemon.service

# systemd reload and starting deamon
systemctl --user daemon-reload
systemctl --user enable --now Audio-Sensei-daemon.service

