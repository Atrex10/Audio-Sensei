#!/bin/bash

# preparing destination folders
mkdir /opt/
mkdir /etc/
mkdir /etc/SAC-daemon/

# copying source code
cp -r ./ /opt/SAC-daemon/

# copying mapping file 
cp ./mapping.json /etc/SAC-daemon/mapping.json

# copying systemd service unit
cp ./SAC-daemon.service /etc/systemd/system/SAC-daemon.service

echo "Files copied"

# creating user for the daemon
useradd -r -s /bin/false SAC-daemon

# setting the owner of the source code
chown -R SAC-daemon:SAC-daemon /opt/SAC-daemon

# setting mapping file to config only
chown -R SAC-daemon:SAC-daemon /etc/SAC-daemon
chmod 640 /etc/SAC-daemon/mapping.json

echo "Privileges setup done"

echo "Instalation complete"

echo "Starting daemon"
# systemd reload and starting deamon
systemctl daemon-reload
systemctl enable --now SAC-daemon.service

