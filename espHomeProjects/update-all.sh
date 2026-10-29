#!/usr/bin/env bash
set -u

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v esphome >/dev/null 2>&1; then
  echo "ESPHome CLI not found. Activate the project's .venv or install ESPHome first." >&2
  exit 127
fi

failed=0

update_device() {
  local config="$1"
  echo "Updating $(basename -- "$config")"
  esphome update-all "$config" || failed=1
}

update_device "$script_dir/cancelloControl.yaml"
update_device "$script_dir/insegnaO.yaml"
update_device "$script_dir/portoneControl.yaml"
#update_device "$script_dir/powerDisplayRing.yaml"
update_device "$script_dir/powerDisplaySoldered.yaml"
update_device "$script_dir/speakerA.yaml"
update_device "$script_dir/stufaControl.yaml"
#update_device "$script_dir/waterLevelPompe.yaml"

if ((failed)); then
  echo "One or more device updates failed." >&2
  exit 1
fi

echo "All device updates succeeded."
