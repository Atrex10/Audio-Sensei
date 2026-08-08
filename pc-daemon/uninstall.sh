#!/bin/bash

# removing user
systemctl disable SAC-daemon.service
systemctl stop SAC-daemon.service

userdel SAC-daemon

# removing files
rm -rf /opt/SAC-daemon/

rm -rf /etc/SAC-daemon/

rm -f /etc/systemd/system/SAC-daemon.service