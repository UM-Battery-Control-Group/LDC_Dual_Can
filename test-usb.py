import usb.core

devices = list(usb.core.find(find_all=True))
print(f"Found {len(devices)} devices")

for d in devices:
    print(hex(d.idVendor), hex(d.idProduct))