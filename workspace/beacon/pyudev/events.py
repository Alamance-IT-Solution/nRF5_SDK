"""
Module for handling events related to devices using pyudev.
"""

from time import sleep
from recognition import device_recognizer
from recognition import devname
import pyudev


def handle_event(action, device):
    """
    Handles events related to devices and prints information about the detected devices.

    Args:
        action (str): The action associated with the event.
        device (pyudev.Device): The pyudev Device object representing the device involved in the event.
    """
    if device is not None:
        dev_type, types = device_recognizer(device, verbose=False)

        if dev_type is not None and devname(device) is not None:
            print(f"Detected event involving: {dev_type}")


# Monitor devices
monitor = pyudev.Monitor.from_netlink(context=pyudev.Context())
observer = pyudev.MonitorObserver(monitor, handle_event)
observer.start()

try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    observer.stop()
