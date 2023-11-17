from time import sleep
from recognition import device_recognizer
from recognition import devname
import pyudev


def handle_event(action, device):
    if device != None:
        dev_type, types = device_recognizer(device, verbose=False)

        if dev_type is not None and devname(device) is not None:
            print(f"Detected event involving: {dev_type}")


# Monitor devices
monitor = pyudev.Monitor.from_netlink(context = pyudev.Context())
observer = pyudev.MonitorObserver(monitor, handle_event)
observer.start()

try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    observer.stop()

