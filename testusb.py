# # import libusb_package
# import usb.core
# from usb.backend import libusb1
# # for dev in libusb_package.find(find_all=True):
# #     print(dev)

# be = usb.backend.libusb1.get_backend(find_library=lambda x: r"C:\Users\siegeljb\Documents\LDC_Dual_Can\libusb-1.0.dll")
    
# #be = libusb1.get_backend()
# print(be)
# #dev = usb.core.find()

# dev = usb.core.find(backend=be,idVendor=0x1D50, idProduct=0x606F)
# print(dev)

import platform

print(platform.architecture())
import os
os.environ['PYUSB_DEBUG'] = 'debug'
import usb.core
usb.core.find()