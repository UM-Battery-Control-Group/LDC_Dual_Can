import ics

def receive_can(device):    
    msgs, error_count = ics.get_messages(device)
    if(len(msgs)):
        print("Received {} messages with {} errors.".format(len(msgs), error_count))
        for i, m in enumerate(msgs):
            print('Message #{}\t'.format(i+1), end='')
            print('\tArbID: {}\tData: {}'.format(hex(m.ArbIDOrHeader), [hex(x) for x in m.Data]))

devices = ics.find_devices()
for device in devices:
     print(device.Name, device.SerialNumber)
ics.open_device(devices[0])
device=devices[0]

while True:
    receive_can(device)



    