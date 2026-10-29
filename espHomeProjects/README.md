# ESPHome devices

## Install ESPHome

ESPHome needs Python 3.12 or newer. From this folder, create a virtual environment and install the ESPHome command-line package:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip esphome
```

Create `secrets.yaml` in this folder with your Wi-Fi and API secrets. Use `secretsTemplate.yaml` as a reference; `.gitignore` keeps `secrets.yaml` out of Git.

## Validate, build, and run a device

Activate the environment first with `source .venv/bin/activate`. Replace `waterLevelPompe.yaml` with the device configuration you want:

```bash
esphome config waterLevelPompe.yaml   # validate configuration
esphome compile waterLevelPompe.yaml  # build firmware only
esphome run waterLevelPompe.yaml      # build, upload (OTA or USB), then show logs
```

For an explicit OTA upload, use `esphome run waterLevelPompe.yaml --device OTA --no-logs`. The device must be online and reachable for OTA; first-time installs may require USB.

## Update all devices

With `.venv` active, run:

```bash
./update-all.sh
```

The script passes all device YAML files in this folder to ESPHome's `update-all` command. ESPHome builds and attempts an OTA upload for each configuration, continues when a device is unavailable, and reports successes and failures. `secretsTemplate.yaml` is excluded. The devices must already have ESPHome OTA enabled and be reachable over the network.
