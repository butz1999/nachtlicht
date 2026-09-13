# Dilbert

Dilbert is the interactive Jupyter tool for controlling Nachtlicht and
observing its Zenoh events. It runs in its own Python environment under WSL;
JupyterLab is opened in the Windows browser.

## Setup

Run the following commands from the repository root:

```bash
cd dilbert
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
cp config.example.py config.py
```

Edit `config.py` and set `ROUTER_LOCATOR` to the TCP address printed by the
Windows Zenoh router. The file is local and intentionally ignored by Git.

## Start

With the environment activated, run:

```bash
jupyter lab --no-browser
```

Open the URL printed by JupyterLab in a Windows browser and open
`dilbert.ipynb`. The notebook opens a Zenoh session to the configured router.
Its button publishes `next` on `nachtlicht/led/color/next`; the GUI also
subscribes to the colour-change and brightness events sent by the ESP32.

The brightness-event counter is a live heartbeat. It increases only while the
notebook and the ESP32 are connected to the Zenoh router; Zenoh Pub/Sub does
not replay events sent before Dilbert subscribed.
