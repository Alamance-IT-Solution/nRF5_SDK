#!/usr/bin/python3

import pyudev
from time import sleep
# import DeviceController


verbose = True

def event_details(action, device):
    properties = device.properties
    print(f"{action} {device.device_node}")
    for key, value in properties.items():
        print(f"  {key}: {value}")

def check_circuitpy(device):
    if device.get('ID_FS_LABEL') == 'CIRCUITPY' or device.get('ID_FS_LABEL_ENC') == 'CIRCUITPY':
        if verbose:
            print(f"  CircuitPython device found at {device.device_node}")
        return True
    else:
        return False


def check_serial(device):

    subsys = device.get('SUBSYSTEM')
    if subsys != None:
        if device.get('SUBSYSTEM') == 'tty':
            """
            if verbose:
                print(f"  Serial device found at {device.device_node}")
            """
            return True

    devname = device.get('DEVNAME')
    if devname != None:
        if device.get('DEVNAME').startswith('/dev/tty'):
            if verbose:
                print(f"  Serial device found at {device.device_node}")
            return True

    return False


def check_nordic_dongle(action, device):
    if check_serial(device):
        if False:
            event_details(action, device)
        else:
            if device.get('ID_VENDOR_ID') == '1915' and device.get('ID_MODEL_ID') == '521f':
                if verbose:
                    print(f"Nordic dongle found at {device.device_node}")
                return True
    return False

def devname(device):
    if device.get('DEVNAME') != None:
        return device.get('DEVNAME')
    else:
        raise Exception("Device name not found")



def handle_event(action, device):
    """
    if verbose:
        print("\n")
        print(f"{action} {device.device_node}")

    """

    if device != None:
        """
        if verbose:
            event_details(action, device)

        if check_circuitpy(device):
            # TODO: Track device name in a database
            print(devname(device))

        if check_serial(device):
            # TODO: Track device name in a database
            print(devname(device))
        """

        if check_nordic_dongle(action, device):
            print(devname(device))


# MAIN
context = pyudev.Context()

# Monitor devices
monitor = pyudev.Monitor.from_netlink(context)
observer = pyudev.MonitorObserver(monitor, handle_event)
observer.start()

try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    observer.stop()
