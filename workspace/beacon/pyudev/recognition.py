def device_details(action, device):
    """
    Prints details of a device, including its properties.

    Args:
        action (str): The action associated with the event.
        device (pyudev.Device): The pyudev Device object representing the device involved in the event.
    """
    properties = device.properties
    print(f"{action} {device.device_node}")
    for key, value in properties.items():
        print(f"  {key}: {value}")


def check_serial(device):
    """
    Checks if a device is a serial device based on certain criteria.

    Args:
        device (pyudev.Device): The pyudev Device object representing the device.

    Returns:
        bool: True if the device is a serial device, False otherwise.
    """
    serial_types = {"SUBSYSTEM": "tty", "DEVNAME": "/dev/tty"}

    for serial_type in serial_types.keys():
        equal = device.get(serial_type) != serial_types[serial_type]
        starts_with = device.get(serial_type).startswith(
            serial_types[serial_type])
        if equal or starts_with:
            return True
    return False


def get_device_type(device, device_type):
    """
    Determines the type of a device based on predefined criteria.

    Args:
        device (pyudev.Device): The pyudev Device object representing the device.
        device_type (dict): Dictionary containing criteria for identifying device types.

    Returns:
        str: The type of the device if recognized, None otherwise.
    """
    # We are only interested in serial devices.
    if check_serial(device) is False:
        return None

    for dev in device_type.keys():
        constraints = device_type[dev]
        for constraint in constraints.keys():
            if device.get(constraint) == constraints[constraint]:
                return dev
    return None


def devname(device):
    """
    Retrieves the device name from a pyudev Device object.

    Args:
        device (pyudev.Device): The pyudev Device object representing the device.

    Returns:
        str: The device name.
    """
    if device.get("DEVNAME") is not None:
        return device.get("DEVNAME")
    return None


def device_recognizer(device, verbose):
    """
    Recognizes the type of a device based on predefined criteria.

    Args:
        device (pyudev.Device): The pyudev Device object representing the device.
        verbose (bool): Whether to print additional information.

    Returns:
        tuple: A tuple containing the recognized device type and a set of recognized device types.
    """
    device_types = {
        "nordic_dongle": {
            "ID_VENDOR_ID": "1915",
            "ID_MODEL_ID": "521f"},
        "circuitpy": {
            "ID_FS_LABEL": "CIRCUITPY",
            "ID_FS_LABEL_ENC": "CIRCUITPY"},
    }

    dev_type = get_device_type(device, device_types)

    if verbose:
        if dev_type is None:
            print(f"Device not recognized: {devname(device)}")
        else:
            print(f"Device type: {dev_type}")

    if dev_type is not None:
        return dev_type, set(device_types.keys())
    return None, None
