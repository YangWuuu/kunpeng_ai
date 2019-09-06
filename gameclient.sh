#!/bin/bash
cd "$(dirname "$0")"
cd bin
chmod +x ai_yang
./ai_yang "$1" "$2" "$3"
ZIP_FILE="/var/log/battle_yangwuuu.zip"
DATE=$(date +%Y_%m_%d_%H_%M_%S)
zip -9 $ZIP_FILE /var/log/battle_yangwuuu*.log
cmd="sshpass -p yc45j scp -o StrictHostKeyChecking=no ${ZIP_FILE} root@119.3.167.104:/root/log/battle_${DATE}.zip"
$cmd
