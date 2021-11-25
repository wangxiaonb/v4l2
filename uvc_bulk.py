import usb.core
import usb.util
import time
import os


def get_usb_device(vid, pid):
    # find our device
    dev = usb.core.find(idVendor=vid, idProduct=pid)

    # was it found?
    if dev is None:
        raise ValueError('Device not found')

    # set the active configuration. With no arguments, the first
    # configuration will be the active one
    dev.set_configuration()

    print(dev)

    # get an endpoint instance
    cfg = dev.get_active_configuration()
    intf = cfg[(0, 0)]

    ep_out = usb.util.find_descriptor(
        intf,
        # match the first OUT endpoint
        custom_match=lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_OUT)

    assert ep_out is not None

    ep_in = usb.util.find_descriptor(
        intf,
        # match the first IN endpoint
        custom_match=lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN)

    assert ep_in is not None

    # msg = bytes([0x01, 0x02, 0x03, 0x04, 0x05])
    # ret = dev.ctrl_transfer(0x40, 0x11, 0x12, 0x13, msg)

    # msg = bytes([0xB3, 0x12])
    # ret = dev.ctrl_transfer(0x40, 0xB3, 0, 0, msg)

    # ret = dev.ctrl_transfer(0x40, 0xB3, 0, 0, msg)
    return (ep_in, ep_out)

 #####################################################################################

if __name__ == '__main__':
    os.system('sudo chown pi:pi /dev/bus/usb/001/*')
    time.sleep(0.2)
    ep_in, ep_out = get_usb_device(0x04B4, 0x00F9)

    ep_out.write([0,0,0], 10000)
    response = ep_in.read(3, 10000)

