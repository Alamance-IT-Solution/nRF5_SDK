"""
Module for finding known devices using pyudev.
"""

from recognition import device_recognizer
import pyudev


def handle_existing_devices(context):
    """
    Handles existing devices, recognizes their types, and returns a dictionary of found devices.

    Args:
        context (pyudev.Context): The pyudev Context object.

    Returns:
        dict: A dictionary containing recognized device types and associated devices.
    """
    devices_found = {}
    for device in context.list_devices():
        if device is not None:
            dev_type, types = device_recognizer(device, verbose=False)

            if dev_type is not None:
                if dev_type not in devices_found.keys():
                    devices_found[dev_type] = [device]
                else:
                    devices_found[dev_type].append(device)
    return devices_found


# Check for existing devices
print(handle_existing_devices(context=pyudev.Context()))
