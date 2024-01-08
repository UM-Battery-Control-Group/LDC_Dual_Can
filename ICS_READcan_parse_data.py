from unicodedata import decimal
import ics

def receive_can(device):
    base_address=0x30    
    msgs, error_count = ics.get_messages(device)
    if(len(msgs)):
        #print("Received {} messages with {} errors.".format(len(msgs), error_count))
        for i, m in enumerate(msgs):
            if m.ArbIDOrHeader==base_address+3:
                print('\t Coil3: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(m.Data[0:4],'little'),int.from_bytes(m.Data[4:6],'little'),int.from_bytes(m.Data[6:8],'little')))
                #print('Message #{}\t'.format(i+1), end='')
                #print('\tArbID: {}\tData: {}'.format(hex(m.ArbIDOrHeader), [hex(x) for x in m.Data]))
            if m.ArbIDOrHeader==base_address:
                print('\t Coil0: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(m.Data[0:4],'little'),int.from_bytes(m.Data[4:6],'little'),int.from_bytes(m.Data[6:8],'little')))    
            if m.ArbIDOrHeader==base_address+1:
                print('\t Coil1: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(m.Data[0:4],'little'),int.from_bytes(m.Data[4:6],'little'),int.from_bytes(m.Data[6:8],'little')))    
            if m.ArbIDOrHeader==base_address+2:
                print('\t Coil2: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(m.Data[0:4],'little'),int.from_bytes(m.Data[4:6],'little'),int.from_bytes(m.Data[6:8],'little')))    
            if m.ArbIDOrHeader==base_address+4:
                print('\t Temperature: {}  \t RH: {} '.format((int.from_bytes(m.Data[0:2],'little')/65536.0)*165.0-40.0,int.from_bytes(m.Data[2:4],'little')/65536.0*100.0))    
            if m.ArbIDOrHeader==base_address+5:
                print('\t Sensor Status: {}  '.format(int.from_bytes(m.Data[0:4],'little') ))    
            if m.ArbIDOrHeader==base_address+6:
                print('\t Sensor Status 0: {} \t Sensor Status 1: {} \t Sensor Status 2: {} \t Sensor Status 3: {} '.format(int.from_bytes(m.Data[0:2],'little'),int.from_bytes(m.Data[2:4],'little'),int.from_bytes(m.Data[4:6],'little'),int.from_bytes(m.Data[6:8],'little') ))    

devices = ics.find_devices()
for device in devices:
     print(device.Name, device.SerialNumber)
ics.open_device(devices[0])
device=devices[0]

while True:
    receive_can(device)
    