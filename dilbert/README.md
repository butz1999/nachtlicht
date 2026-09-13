# Dilbert

Dilbert is the interactive Jupyter tool for sending Zenoh messages to
Nachtlicht. It runs in its own Python environment under WSL; JupyterLab is
opened in the Windows browser.

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
`dilbert.ipynb`. The notebook opens a Zenoh session to the configured router
and publishes `next` on `nachtlicht/led/color/next`.

The first notebook version verifies the host-side publisher. The firmware
subscriber is added separately; only after that does the button cell cause a
visible colour change on the ESP32-S3.
