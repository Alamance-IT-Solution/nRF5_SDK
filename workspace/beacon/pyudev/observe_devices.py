#!/usr/bin/python3

import pyudev
from time import sleep

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
            return True

    devname = device.get('DEVNAME')
    if devname != None:
        if device.get('DEVNAME').startswith('/dev/tty'):
            return True

    return False

def check_nordic_dongle(device):
    if check_serial(device):
        if device.get('ID_VENDOR_ID') == '1915' and device.get('ID_MODEL_ID') == '521f':
            if verbose:
                print(f"Nordic dongle found at {device.device_node}")
            return True
    return False

def devname(device):
    if device.get('DEVNAME') is not None:
        return device.get('DEVNAME')
    else:
        raise Exception("Device name not found")


def device_recognizer(device):
    if device is None:
        raise Exception("Device is None")

    if check_nordic_dongle(device):
        print(f"Nordic dongle found at {devname(device)}")

    if check_circuitpy(device):
        print(devname(device))

def handle_event(action, device):
    if device != None:
        device_recognizer(device)

def handle_existing_devices():
    for device in context.list_devices():
        if device is not None:
            device_recognizer(device)

# MAIN
context = pyudev.Context()

# Check for existing devices
handle_existing_devices()

# Monitor devices
monitor = pyudev.Monitor.from_netlink(context)
observer = pyudev.MonitorObserver(monitor, handle_event)
observer.start()

try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    observer.stop()

