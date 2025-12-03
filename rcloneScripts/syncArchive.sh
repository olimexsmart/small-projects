#!/bin/bash

rclone sync -P /media/teradisk/FilesArchiveRoot onedrive:BackupArchivioNAS &&
echo "DONE ARCHIVE"