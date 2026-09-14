# Dilbert

Dilbert is the interactive Jupyter tool for controlling Nachtlicht and
observing its Zenoh events. The notebook remains in the WSL repository, while
JupyterLab runs in its own native Windows Python environment.

## Setup

Run the following commands from PowerShell. The virtual environment is kept on
the Windows filesystem because a Windows Python virtual environment inside the
`\\wsl.localhost` share does not reliably discover installed packages.

```powershell
$dilbertDir = "\\wsl.localhost\ubuntu\home\andi\git\nachtlicht\dilbert"
py -3.14 -m venv "$env:LOCALAPPDATA\Nachtlicht\dilbert-venv"
& "$env:LOCALAPPDATA\Nachtlicht\dilbert-venv\Scripts\Activate.ps1"
python -m pip install --upgrade pip
python -m pip install -r "$dilbertDir\requirements.txt"
```

If an existing environment was created before the native-extension requirements
were updated, refresh all dependencies using Windows binary wheels:

```powershell
python -m pip install --upgrade --force-reinstall --only-binary=:all: -r "$dilbertDir\requirements.txt"
```

No router address is configured: Dilbert discovers a local Zenoh router via
UDP multicast scouting on `224.0.0.224:7446`.

## Start

From WSL, run the helper:

```bash
./scripts/jupyter.sh
```

It starts the native Windows environment and prints the JupyterLab URL. Open
that URL in a Windows browser and open
`dilbert.ipynb`. The notebook scouts for a local Zenoh router and opens a
Zenoh session using the locator announced by that router.
Its button publishes `next` on `nachtlicht/led/color/next`; the GUI also
subscribes to the colour-change and brightness events sent by the ESP32.

The brightness-event counter is a live heartbeat. It increases only while the
notebook and the ESP32 are connected to the Zenoh router; Zenoh Pub/Sub does
not replay events sent before Dilbert subscribed.
