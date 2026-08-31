#!/bin/bash
set -euo pipefail

PROJECT_DIR="/home/olli/piScreenCameras"
VENV_PYTHON="/home/olli/screen-venv/bin/python"

exec "$VENV_PYTHON" "$PROJECT_DIR/display_camera_buttons.py"
