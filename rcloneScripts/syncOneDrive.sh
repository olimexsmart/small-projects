#!/bin/bash

sqlite3 /home/olli/.memos/memos_prod.db ".backup '/home/olli/FileSyncRoot/Documenti/memos.sqlite'"
rclone sync -P /home/olli/FileSyncRoot onedrive:BackupNAS &&
echo "DONE STANDARD"

rclone sync -P /home/olli/homeAssistant/backups onedrive:BackupHomeAssistant
echo "DONE Home Assistant"