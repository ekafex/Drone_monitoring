## RTEC digiBASE-RH Python acquisition setup

### 1. Linux / Debian / Raspberry Pi OS

Install system USB support and Python tools:

```bash
sudo apt update
sudo apt install python3 python3-venv python3-pip libusb-1.0-0
```

Create an environment:

```bash
python3 -m venv ~/venvs/digibase
source ~/venvs/digibase/bin/activate
pip install --upgrade pip
pip install digibase numpy matplotlib pyusb
```

Create the firmware directory:

```bash
mkdir -p ~/.digiBase
```

Copy ORTEC firmware files there:

```bash
cp digiBase.rbf ~/.digiBase/
cp digiBaseRH.rbf ~/.digiBase/
```

Set USB permissions:

```bash
sudo nano /etc/udev/rules.d/99-ortec-digibase.rules
```

Add:

```text
SUBSYSTEM=="usb", ATTR{idVendor}=="0a2d", ATTR{idProduct}=="001f", MODE="0666", GROUP="plugdev"
```

Reload:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo usermod -aG plugdev $USER
```

Then unplug/replug the digiBASE and log out/in.

Check:

```bash
lsusb
```

Expected:

```text
0a2d:001f ORTEC digiBASE-RH
```

### 2. Windows

There are two routes.

For **official ORTEC software**, install ORTEC CONNECTIONS/MAESTRO and use the vendor driver. The digiBASE manual describes installation through CONNECTIONS Driver Update Kit, MAESTRO, and MCB Configuration. ([ManualsLib](https://www.manualslib.com/manual/1990334/Ametek-Ortec-Digibase.html?utm_source=chatgpt.com))

For **Python/libusb access**, install:

```powershell
python -m pip install digibase numpy matplotlib pyusb
```

Then copy:

```text
digiBase.rbf
digiBaseRH.rbf
```

to a convenient folder, for example:

```text
C:\Users\<your-user>\.digiBase\
```

If Python cannot open the USB device, Windows likely needs a generic USB driver such as **WinUSB** or **libusbK** installed for the digiBASE interface. Zadig is commonly used for this purpose; it installs generic USB drivers such as WinUSB/libusbK for libusb-based applications. ([AlternativeTo](https://alternativeto.net/software/zadig/about/?utm_source=chatgpt.com))

Important: replacing the ORTEC driver with WinUSB/libusbK may stop MAESTRO from seeing the device until you revert the driver.

## Minimal spectrum acquisition script

Save as `digibase_take_spectrum.py`:

```python
#!/usr/bin/env python3

import argparse
import time
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

from digibase import digiBase


def main():
    parser = argparse.ArgumentParser(
        description="Acquire one PHA spectrum from an ORTEC digiBASE/digiBASE-RH."
    )

    parser.add_argument("--hv", type=float, default=600.0,
                        help="PMT high voltage in volts. Start conservatively.")
    parser.add_argument("--time", type=float, default=10.0,
                        help="Acquisition time in seconds.")
    parser.add_argument("--fine-gain", type=float, default=1.0,
                        help="ADC fine gain.")
    parser.add_argument("--lld", type=int, default=24,
                        help="Lower-level discriminator in ADC channels.")
    parser.add_argument("--out", type=str, default="spectrum",
                        help="Output basename without extension.")

    args = parser.parse_args()

    outbase = Path(args.out)

    print("Connecting to digiBASE...")
    base = digiBase()

    print(f"Serial number: {base.serial}")

    print(f"Setting HV = {args.hv} V")
    base.hv = args.hv
    base.hv_enabled = True

    print(f"Setting fine gain = {args.fine_gain}")
    base.fine_gain = args.fine_gain

    print(f"Setting LLD = {args.lld}")
    base.lld = args.lld

    print("Setting PHA mode")
    base.set_acq_mode_pha()

    print("Clearing previous spectrum if supported...")
    try:
        base.clear()
    except Exception:
        pass

    print("Starting acquisition...")
    base.start()

    time.sleep(args.time)

    print("Stopping acquisition...")
    base.stop()

    spectrum = np.asarray(base.spectrum, dtype=np.int64)
    channels = np.arange(len(spectrum))

    txt_file = outbase.with_suffix(".txt")
    csv_file = outbase.with_suffix(".csv")
    png_file = outbase.with_suffix(".png")

    np.savetxt(txt_file, spectrum, fmt="%d")

    data = np.column_stack([channels, spectrum])
    np.savetxt(
        csv_file,
        data,
        delimiter=",",
        header="channel,counts",
        comments="",
        fmt=["%d", "%d"],
    )

    plt.figure()
    plt.plot(channels, spectrum)
    plt.xlabel("Channel")
    plt.ylabel("Counts")
    plt.title(f"digiBASE spectrum, {args.time:g} s, HV={args.hv:g} V")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(png_file, dpi=150)
    plt.show()

    print()
    print("Acquisition complete.")
    print(f"Channels:     {len(spectrum)}")
    print(f"Total counts: {spectrum.sum()}")
    print(f"Peak channel: {spectrum.argmax()}")
    print(f"Saved:        {txt_file}")
    print(f"Saved:        {csv_file}")
    print(f"Saved:        {png_file}")


if __name__ == "__main__":
    main()
```

Run:

```bash
python digibase_take_spectrum.py --hv 600 --time 30 --fine-gain 1.0 --lld 24 --out test_30s
```

This produces:

```text
test_30s.txt
test_30s.csv
test_30s.png
```

Start with a safe low HV, then increase only according to your PMT/scintillator specifications.
