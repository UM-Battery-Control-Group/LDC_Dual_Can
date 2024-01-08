from unicodedata import decimal
import numpy as np
import time
import asyncio
import functools
import sys
from typing import List
import pandas as pd
import can
from can.notifier import MessageRecipient
import usb
# from PySide6.QtWidgets import (
#     QWidget,
#     QLabel,
#     QLineEdit,
#     QTextEdit,
#     QPushButton,
#     QVBoxLayout,
#     QListWidget,
#     QGridLayout,
# )
from PySide6.QtWidgets import *
from PySide6 import QtCore, QtGui
import qasync
from qasync import asyncSlot, asyncClose, QApplication
import os
import time 
from time import sleep
from datetime import datetime
import csv
import copy
import logging
#\VID_1D50&PID_606F&MI_00\7&1D200665&0&0000
# libwdi:debug [wdi_create_list] Device description: 'USB2CAN v1 (Interface 0)'
# libwdi:debug [wdi_create_list] Hardware ID: USB\VID_1D50&PID_606F&REV_0000&MI_01
# libwdi:debug [wdi_create_list] Compatible ID: USB\MS_COMP_WINUSB
# libwdi:debug [wdi_create_list] Driver version: 10.0.19041.1
# libwdi:debug [wdi_create_list] WINUSB USB device (3): USB
#dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
logging.basicConfig(level=logging.INFO)
heartbeatstruct={0:'|',1:'/',2:'--',3:'\\'}
## Set the Config Here
## Set 1 for Chamber A and 2 for Chambers B & C
config = 1
if config == 1:
    N_channels = 40
    start_idx =  0
elif config == 2:
    N_channels = 80
    start_idx =  40
elif config == 3:
    N_channels = 120
    start_idx =  0
BattCelldata = [{'Datapoint Number':0,'Test Time':0,'Current':0,'Potential':0,'Timestamp':int(time.time()*1000),'lastData':int(time.time()*1000),'LDC SENSOR':0,'LDC REF':0,'Ambient Temperature':0,'Ambient RH':0,'LDC N':0,'LDC STD':0,'REF N':0,'REF STD':0,'LDC scaled':0,'LDC status':0,'REF status':0,'Filename':'editme','StartTime':int(time.time()*1000),'Newdata':0,'LogStatus':'Closed','DriveCurrent':0,'DriveCurrentRef':0,'LDCStatus':0,'NAHwarn':0,'NAHwarnRef':0,'heartbeat':0} for i in range(start_idx,start_idx+N_channels)]
File_Prefix='E:\\VDFData\\'

# target VDF format.
# Start Time: 1657721407755
# Timezone: America/New_York
# Channel Number: 31
# [DATA START]
# Datapoint Number	Test Time	Current	Potential	Timestamp	LDC SENSOR	LDC REF	Temperature	Ambient Temperature	Ambient RH	LDC N	 LDC STD	REF N	REF STD	LDC scaled
# none	second	amp	volt	epoch	none	none	celsius	celsius	percent	none	none	none	none	none
# 562	5.000335	0.000000	0.000000	1657721410095	9601203	13672657	26.87	25.2	26.9	99.000000	98.000000	99.000000	324.000000	4290895842.000000
# 563	9.999318	0.000000	0.000000	1657721415095	9601198	13672751	26.89	25.2	26.8	99.000000	88.000000	99.000000	404.000000	4290895743.000000
# 564	15.000259	0.000000	0.000000	1657721420095	9601248	13672698	26.88	25.2	26.8	99.000000	300.000000	99.000000	402.000000	4290895846.000000
# 565	20.000117	0.000000	0.000000	1657721425095	9601188	13672739	26.87	25.2	26.8	99.000000	102.000000	99.000000	392.000000	4290895745.000000
# 566	25.000235	0.000000	0.000000	1657721430095	9601206	13672770	26.87	25.3	26.8	99.000000	100.000000	99.000000	394.000000	4290895732.000000
# 567	30.000603	0.000000	0.000000	1657721435095	9601172	13672721	26.88	25.2	26.8	99.000000	90.000000	99.000000	347.000000	4290895747.000000

#logging_dict={}
dfdc = pd.read_csv("drive_current.csv")
logging_dict=pd.read_excel("neware LDC data logging.xlsx")
if config == 2:
    logging_dict = logging_dict.iloc[start_idx:,:]
    logging_dict = logging_dict.reset_index()
# Channel #	PROJECT	Cell #	Neware rack	Neware Channel	Thermal Chamber	Thermal Chamber rack (top is 1	Temp	Pressure	Test Protocol	Can Address (HEX)	Can Sub	Reference	index	Temperature msg ID	Ref index   TestName
# 1	GMJuly2022	12	36	1-1	A	1	25	5	0	10	1	3	17	20	19
# 2	GMJuly2022	15	36	1-2	A	1	25	5	0	10	0	3	16	20	19
# 3	GMJuly2022	44	36	1-3	A	1	25	5	0	10	2	3	18	20	19


# file = open("/home/pi/data_log.csv", "a")
# i=0
# if os.stat("/home/pi/data_log.csv").st_size == 0:
#         file.write("Time,Sensor1,Sensor2,Sensor3,Sensor4,Sensor5\n")
# while True:
#         i=i+1
#         now = datetime.now()
#         file.write(str(now)+","+str(i)+","+str(-i)+","+str(i-10)+","+str(i+5)+","+str(i*i)+"\n")
#         file.flush()
#         time.sleep(5)<br>file.close()

class CustomDialog(QDialog):
    def __init__(self,canbus,dc_can_id):
        super().__init__()
        self.canbus=canbus
        self.dc_can_id = dc_can_id
        self.dc_ch_idx = np.ones(4)
        #dfs = dfdc.loc[dfdc['can_id'] == self.idx]
        dfs = dfdc.loc[dfdc['can_id'] == self.dc_can_id]
        dfs = dfs.reset_index()
        self.dc_ch_idx[0]=dfs.dc_0[0]
        self.dc_ch_idx[1]=dfs.dc_1[0]
        self.dc_ch_idx[2]=dfs.dc_2[0]
        self.dc_ch_idx[3]=dfs.dc_3[0]
       
        self.setWindowTitle("Drive Current for ID 0x{:x}".format(dc_can_id) )

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.reject)

        self.dlayout = QGridLayout()
        self.ll = QLabel("Set Drive Current Here")
        self.lc0 = QLabel("Sub 0:")
        self.lc1 = QLabel("Sub 1:")
        self.lc2 = QLabel("Sub 2:")
        self.lc3 = QLabel("Sub 3:")

        self.sp0 = QSpinBox()
        self.sp0.setMinimum(4)
        self.sp0.setMaximum(15)
        self.sp1 = QSpinBox()
        self.sp1.setMinimum(4)
        self.sp1.setMaximum(15)
        self.sp2 = QSpinBox()
        self.sp2.setMinimum(4)
        self.sp2.setMaximum(15)
        self.sp3 = QSpinBox()
        self.sp3.setMinimum(4)
        self.sp3.setMaximum(15)
        self.sp0.valueChanged.connect(self.vc0)
        self.sp1.valueChanged.connect(self.vc1)
        self.sp2.valueChanged.connect(self.vc2)
        self.sp3.valueChanged.connect(self.vc3)
        self.sp0.setValue(self.dc_ch_idx[0])
        self.sp1.setValue(self.dc_ch_idx[1])
        self.sp2.setValue(self.dc_ch_idx[2])
        self.sp3.setValue(self.dc_ch_idx[3])

        ix=0
        iy=0
        self.dlayout.addWidget(self.ll,ix,iy,1,4)
        ix+=1
        self.dlayout.addWidget(self.lc0,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.sp0,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.lc1,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.sp1,ix,iy)
        ix+=1
        iy=0
        self.dlayout.addWidget(self.lc2,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.sp2,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.lc3,ix,iy)
        iy+=1
        self.dlayout.addWidget(self.sp3,ix,iy)
        ix+=1
        iy=0
        self.dlayout.addWidget(self.buttonBox,ix,iy,1,4)
        self.setLayout(self.dlayout)
    
    def accept(self):
        print("dc values")
        print(self.dc_ch_idx)
        dfdc.loc[dfdc['can_id'] == self.dc_can_id,"dc_0"]=self.dc_ch_idx[0]
        dfdc.loc[dfdc['can_id'] == self.dc_can_id,"dc_1"]=self.dc_ch_idx[1]
        dfdc.loc[dfdc['can_id'] == self.dc_can_id,"dc_2"]=self.dc_ch_idx[2]
        dfdc.loc[dfdc['can_id'] == self.dc_can_id,"dc_3"]=self.dc_ch_idx[3]
        dfdc.to_csv("drive_current.csv",index=False)

          # Reset the drive current
        msg = can.Message(arbitration_id=int(self.dc_can_id)+0xA, data=[int(self.dc_ch_idx[0]),  int(self.dc_ch_idx[1]), int(self.dc_ch_idx[2]), int(self.dc_ch_idx[3]),0, 0, 0, 0 ], is_extended_id=False)
        
        try:
            self.canbus.send(msg)
            print(f"Message sent on {self.canbus.channel_info}")
            #print(self.canbus.)
        except can.CanError:
            print("Message NOT sent")

        super().accept()
    
    def vc0(self):
        self.dc_ch_idx[0]=self.sp0.value()
    def vc1(self):
        self.dc_ch_idx[1]=self.sp1.value()
    def vc2(self):
        self.dc_ch_idx[2]=self.sp2.value()
    def vc3(self):
        self.dc_ch_idx[3]=self.sp3.value()


class MainWindow(QMainWindow):
    def __init__(self,canbus):
        super().__init__()
        self.canbus=canbus
        self.setWindowTitle("Neware LDC CAN Data Logger")
        self.input = QLineEdit()
        self.input.setText("proj_cell_xxx_test_xxx.csv")
        # self.input.textChanged.connect(self.label.setText)
        self.l0 = QLabel("Channel #")
        self.l01 = QLabel("Sens")
        self.l21 = QLabel("0")
        self.l02 = QLabel("Ref")
        self.l22 = QLabel("0")
        self.l03 = QLabel("Temp")
        self.l23 = QLabel("0")
        self.l04 = QLabel("RH")
        self.l24 = QLabel("0")
        self.l05 = QLabel("Sens N")
        self.l25 = QLabel("0")
        self.l06 = QLabel("Sens STD")
        self.l26 = QLabel("0")
        self.l07 = QLabel("Ref N")
        self.l27 = QLabel("0")
        self.l08 = QLabel("Ref STD")
        self.l28 = QLabel("0")
        self.l09 = QLabel("Sens Stat : H error")
        self.l29 = QLabel("0")
        self.l10 = QLabel("Ref Stat : H error")
        self.l30 = QLabel("0")
        self.l11 = QLabel("Drive")
        self.l31 = QLabel("0")
        self.l12 = QLabel("TimeStamp : DeltaT")
        self.l32 = QLabel("0")


        self.l1 = QLabel("411")
        self.idx = 0
        self.l2 = QLabel()
        self.b1 = QPushButton("Start")
        self.b2 = QPushButton("Stop")
        self.b3 = QPushButton("Drive Current")
        self.b4 = QPushButton("Reset Canbus")
        self.b1.clicked.connect(self.b1_state)
        self.b2.clicked.connect(self.b2_state)
        self.b3.clicked.connect(self.b3_state)
        self.b4.clicked.connect(self.b4_state)
        self.b2.setEnabled(False)

        self.combobox = QListWidget()
        # self.combobox.addItems(['411','412','413','414','415','416','417','418'])
        self.combobox.addItems(['{:03d} : R{:02d} CH'.format(logging_dict.index[i]+start_idx+1,logging_dict['Neware rack'].values[i])+ logging_dict['Neware Channel'].values[i] + ' \t CELL{:03d}'.format(logging_dict['Cell #'].values[i]) + ' \t BaseID 0x{:x}'.format(logging_dict['Temperature msg ID'].values[i]-4)  + '   Can_Sub {:02d}'.format(logging_dict['Can Sub'].values[i])   for i in range(N_channels)])
   
        #logging_dict.index[i]
        #my_value = 411
        #self.combobox.addItems([str(x) for x in range(my_value,my_value+8)])
        #my_value = 421
        #self.combobox.addItems([str(x) for x in range(my_value,my_value+8)])
        for i in range (N_channels):
            BattCelldata[i]['Filename']=self.set_file_name(i) 

        self.combobox.currentRowChanged.connect(self.activated)
        self.combobox.currentTextChanged.connect(self.text_changed)
        # self.combobox.currentIndexChanged.connect(self.index_changed)

        infolayout = QGridLayout()
        ix=0
        iy=0
        infolayout.addWidget(self.l01,ix,iy)
        iy+=1
        infolayout.addWidget(self.l02,ix,iy)
        iy+=1
        infolayout.addWidget(self.l03,ix,iy)
        iy+=1
        infolayout.addWidget(self.l04,ix,iy)
        iy=0
        ix+=1
        infolayout.addWidget(self.l21,ix,iy)
        iy+=1
        infolayout.addWidget(self.l22,ix,iy)
        iy+=1
        infolayout.addWidget(self.l23,ix,iy)
        iy+=1
        infolayout.addWidget(self.l24,ix,iy)
        iy=0
        ix+=1
        infolayout.addWidget(self.l05,ix,iy)
        iy+=1
        infolayout.addWidget(self.l06,ix,iy)
        iy+=1
        infolayout.addWidget(self.l07,ix,iy)
        iy+=1
        infolayout.addWidget(self.l08,ix,iy)
        iy=0
        ix+=1
        infolayout.addWidget(self.l25,ix,iy)
        iy+=1
        infolayout.addWidget(self.l26,ix,iy)
        iy+=1
        infolayout.addWidget(self.l27,ix,iy)
        iy+=1
        infolayout.addWidget(self.l28,ix,iy)
        iy=0
        ix+=1
        infolayout.addWidget(self.l09,ix,iy)
        iy+=1
        infolayout.addWidget(self.l10,ix,iy)
        iy+=1
        infolayout.addWidget(self.l11,ix,iy)
        iy+=1
        infolayout.addWidget(self.l12,ix,iy)
        iy=0
        ix+=1
        infolayout.addWidget(self.l29,ix,iy)
        iy+=1
        infolayout.addWidget(self.l30,ix,iy)
        iy+=1
        infolayout.addWidget(self.l31,ix,iy)
        iy+=1
        infolayout.addWidget(self.l32,ix,iy)

        layout = QGridLayout()
        ix=0
        iy=0
        layout.addWidget(self.l0,ix,iy)
        ix+=1
        iy=0
        layout.addWidget(self.combobox,ix,iy)
        iy+=1
        layout.addLayout(infolayout,ix,iy)
        iy+=1
        layout.addWidget(self.b3,ix,iy)
        ix+=1
        iy=0
        layout.addWidget(self.l1,ix,iy)
        iy+=1
        layout.addWidget(self.input,ix,iy)
        iy+=1
        layout.addWidget(self.l2,ix,iy)
        iy+=1
        layout.addWidget(self.b1,ix,iy)
        iy+=1
        layout.addWidget(self.b2,ix,iy)
        iy+=1
        layout.addWidget(self.b4,ix,iy)


        self.l2.setText("Stopped")

        layout.setColumnMinimumWidth(1,450)
        layout.setColumnMinimumWidth(0,450)

        # self.lsens.setText(str(time.time()))
        # self.sens_val()
        # timer = QtCore.QTimer()
        # timer.timeout.connect(self.sens_val)
        # timer.start(5000)  # every 10,000 milliseconds
        
        container = QWidget()
        container.setLayout(layout)

        # Set the central widget of the Window.
        self.setCentralWidget(container)

    
    @asyncSlot()
    async def start_log(self):
        text=self.input.text()
        BattCelldata[self.idx]['Filename']="%s" %(text)
        BattCelldata[self.idx]['LogStatus']='Start'
        BattCelldata[self.idx]['Datapoint Number']=0
        BattCelldata[self.idx]['StartTime']=int(time.time()*1000)
        BattCelldata[self.idx]['Timestamp']=BattCelldata[self.idx]['StartTime']
                    
        

        # Headerlist=['Datapoint Number','Test Time','Current','Potential','Timestamp','LDC SENSOR','LDC REF','Ambient Temperature','Ambient RH','LDC N','LDC STD','REF N','REF STD','LDC scaled','LDC status','REF status']
        # SubHeader=['none','second','amp','volt','epoch','none','none','celsius','percent','none','none','none','none','none']
        # timestr = time.strftime("%Y%m%d_%H%M%S")
        # self.BattCelldata[self.idx]['Filename']= 'proj_cell_xxx_test_xxx_'+timestr +'.csv'
        # self.BattCelldata[self.idx]['LogStatus']= 1
        # print('Log Status: ',self.BattCelldata[self.idx]['LogStatus'])
        # with open(self.BattCelldata[self.idx]['Filename'], 'w',newline='') as file:
        #     headerwriter=csv.writer(file,delimiter='\t')
        #     headerwriter.writerow(Headerlist)
        #     headerwriter.writerow(SubHeader)
        self.l2.setText("Running")
        self.combobox.item(self.idx).setBackground(QtCore.Qt.green)
        self.b1.setEnabled(False)
        self.b2.setEnabled(True)
        #self.input.setText(self.BattCelldata[self.idx]['Filename'])
        # Reset the drive current

        dfs = dfdc.loc[dfdc['can_id'] == self.dc_can_id]
        dfs = dfs.reset_index()
        # self.dc_ch_idx[0]=dfs.dc_0[0]
        # self.dc_ch_idx[1]=dfs.dc_1[0]
        # self.dc_ch_idx[2]=dfs.dc_2[0]
        # self.dc_ch_idx[3]=dfs.dc_3[0]
        candata=[int(dfs.dc_0[0]),  int(dfs.dc_1[0]), int(dfs.dc_2[0]), int(dfs.dc_3[0]),0, 0, 0, 0 ]
        # Reset the drive current
        msg = can.Message(arbitration_id=int(self.dc_can_id+0xA),data=candata , is_extended_id=False)

#        msg = can.Message(arbitration_id=logging_dict['Temperature msg ID'].values[self.idx]-4+0xA, data=[0, 0, 0, 0, 0, 0, 0, 0], is_extended_id=False)
        
        try:
            self.canbus.send(msg)
            print(f"Message sent on {self.canbus.channel_info}")
        except can.CanError:
            print("Message NOT sent")
     
    @asyncSlot()
    async def stop_log(self):
        BattCelldata[self.idx]['LogStatus']='Closed' #FixME
        #print('Log Status: ',BattCelldata[self.idx]['LogStatus'])
        self.l2.setText("Stopped")
        self.combobox.item(self.idx).setBackground(QtCore.Qt.white)
        self.b2.setEnabled(False)
        self.b1.setEnabled(True)
        #self.input.setText("proj_cell_xxx_test_xxx.csv")

    @asyncSlot()
    async def activated(self, index):
        for i in range(N_channels):
            if logging_dict['Temperature msg ID'].values[i]-4==logging_dict['Temperature msg ID'].values[self.idx]-4:
                self.combobox.item(i).setForeground(QtCore.Qt.black)
        self.idx=index
        print("Activated index:", index)
        print(logging_dict['Temperature msg ID'].values[self.idx]-4)
        self.dc_can_id = logging_dict['Temperature msg ID'].values[self.idx]-4
        print('Log Status: ',BattCelldata[self.idx]['LogStatus'])
        if BattCelldata[self.idx]['LogStatus']=='Running':
            self.l2.setText("Running")
            self.combobox.item(self.idx).setBackground(QtCore.Qt.green)
            self.b1.setEnabled(False)
            self.b2.setEnabled(True)
            self.input.setText(BattCelldata[self.idx]['Filename'])
        else:
            self.l2.setText("Stopped")
            self.combobox.item(self.idx).setBackground(QtCore.Qt.white)
            self.b2.setEnabled(False)
            self.b1.setEnabled(True)
            #BattCelldata[i]['Filename']= File_Prefix+logging_dict['PROJECT'].values[i]+'_CELL{:03d}'.format(logging_dict['Cell #'].values[i])+'_CH{:03d}'.format(logging_dict.index[i])+'.vdf' # add date,
            
            BattCelldata[self.idx]['Filename']=self.set_file_name(self.idx)
            self.input.setText(BattCelldata[self.idx]['Filename'])
            #self.input.setText("proj_cell_xxx_test_xxx.csv")
        for i in range(N_channels):
            if logging_dict['Temperature msg ID'].values[i]-4==logging_dict['Temperature msg ID'].values[self.idx]-4:
                self.combobox.item(i).setForeground(QtCore.Qt.magenta)

    @asyncSlot()
    async def text_changed(self, s):
        # print("Text changed:", s)
        # self.l1 = QLabel(s)
        self.l1.setText(s)

    # @asyncSlot()
    # async def index_changed(self, index):
    #     print("Index changed", index)
    
    @asyncSlot()
    async def sens_val(self):
        # await asyncio.sleep(1)
        self.l21.setText(str(BattCelldata[self.idx]['LDC SENSOR']))
        self.l22.setText(str(BattCelldata[self.idx]['LDC REF']))
        self.l23.setText(str(BattCelldata[self.idx]['Ambient Temperature']))
        self.l24.setText(str(BattCelldata[self.idx]['Ambient RH']))
        self.l25.setText(str(BattCelldata[self.idx]['LDC N']))
        self.l26.setText(str(BattCelldata[self.idx]['LDC STD']))
        self.l27.setText(str(BattCelldata[self.idx]['REF N']))
        self.l28.setText(str(BattCelldata[self.idx]['REF STD']))
        self.l29.setText(str(BattCelldata[self.idx]['LDC status'])+ " : " +str(BattCelldata[self.idx]['NAHwarn']) )
        self.l30.setText(str(BattCelldata[self.idx]['REF status'])+ " : "+ str(BattCelldata[self.idx]['NAHwarnRef']))
        self.l31.setText(str(BattCelldata[self.idx]['DriveCurrent']))
        self.l32.setText(str(BattCelldata[self.idx]['Timestamp'])+ " : "+ str(BattCelldata[self.idx]['Timestamp']-BattCelldata[self.idx]['lastData'])  )
        # self.lsens.setText(str(time.time()))
        pixmapi = QStyle.StandardPixmap.SP_MessageBoxCritical
        icon = self.style().standardIcon(pixmapi)
        pixmapi1 = QStyle.StandardPixmap.SP_DialogYesButton
        icon1 = self.style().standardIcon(pixmapi1)
        for i in range(N_channels):
            t_diff = (int(time.time()*1000)-BattCelldata[i]['Timestamp'])
            # print(t_diff)
            if t_diff>=10000:
                self.combobox.item(i).setIcon(icon)
            else:
               self.combobox.item(i).setIcon(icon1)
    
    @asyncSlot()
    async def b1_state(self):
        
        # f_name = self.input.text()
        # print(f_name)
        self.start_log()
    
    @asyncSlot()
    async def b2_state(self):
        self.stop_log()

    @asyncSlot()
    async def b3_state(self):
        print("Can ID DC:")
        print(self.dc_can_id)
        self.dlg = CustomDialog(self.canbus,self.dc_can_id)
        self.dlg.open()

    @asyncSlot()
    async def b4_state(self):
        print("Resetting the canbus")
       # print(self.canbus.)


    def set_file_name(self,i):
        # Proposed Naming Convention
        # All tests will follow the naming convention described below.

        # [ProjectName]_[DeviceID]_[TestType]_[ProcedureVersion]_[Temperature]_[Pressure]_[TestDate]_[RunNumber]
        # Examples
        # PEYMAN2020_CELL001_CL_1_P25C_5P0PSI_20220602_R1
        mydatestring=datetime.now().strftime("%Y%m%d")
        int_pres=int(logging_dict['Pressure'].values[i])
        intfrac_pres=int(10*logging_dict['Pressure'].values[i])-10*int_pres
        fname=File_Prefix+logging_dict['PROJECT'].values[i]+'_CELL{:03d}'.format(logging_dict['Cell #'].values[i])+'_Test'+str(logging_dict['Test Protocol'].values[i])+'_1'+'_P{:d}C'.format(logging_dict['Temp'].values[i]) +'_{:d}P{:d}PSI_'.format(int_pres,intfrac_pres) +mydatestring+ '_R0'+'_CH{:03d}'.format(logging_dict.index[i]+start_idx+1)+'.csv' # add date,
        return fname

def parse_msg(msg: can.Message)-> None:
    if(msg.is_error_frame):
        print('got an error frame\n')
        print(msg)

    parsed_data={"ARBID":msg.arbitration_id}
    parsed_data['channel']=0
    #base_address=0x30    
    #msgs, error_count = ics.get_messages(device)
    if msg.arbitration_id % 16==0 or msg.arbitration_id % 16==1 or msg.arbitration_id % 16==2 or msg.arbitration_id % 16==3:
        # we have data from Coil 0,1, or 2
        LDC_count=int.from_bytes(msg.data[0:4],'little')
        LDC_STDEV=int.from_bytes(msg.data[4:6],'little')
        LDC_Nsamples=int.from_bytes(msg.data[6:8],'little')
        flag_standalone=0

        for i in range(N_channels):
            if(logging_dict['index'].values[i]==msg.arbitration_id):
                BattCelldata[i]['LDC SENSOR']=LDC_count
                BattCelldata[i]['LDC N']=LDC_Nsamples
                BattCelldata[i]['LDC STD']=LDC_STDEV
                BattCelldata[i]['lastData']=BattCelldata[i]['Timestamp']
                BattCelldata[i]['Timestamp']=int(time.time()*1000) #msg.timestamp*1000
                BattCelldata[i]['Test Time']=(BattCelldata[i]['Timestamp']- BattCelldata[i]['StartTime'])/1000
                    
                BattCelldata[i]['Datapoint Number']=BattCelldata[i]['Datapoint Number']+1
                flag_standalone=1
            elif(logging_dict['Ref index'].values[i] == msg.arbitration_id):
                BattCelldata[i]['LDC REF']=LDC_count
                BattCelldata[i]['REF N']=LDC_Nsamples
                BattCelldata[i]['REF STD']=LDC_STDEV
    elif(msg.arbitration_id % 16==4):
        LDC_Temperature=(int.from_bytes(msg.data[0:2],'little')/65536.0)*165.0-40.0
        LDC_RH=int.from_bytes(msg.data[2:4],'little')/65536.0*100.0 
        for i in range(N_channels):
            if(logging_dict['Temperature msg ID'].values[i]==msg.arbitration_id):
                BattCelldata[i]['Ambient Temperature']="{:.2f}".format(LDC_Temperature)        
                BattCelldata[i]['Ambient RH']="{:.2f}".format(LDC_RH) 
    elif(msg.arbitration_id % 16==5):
        status=int.from_bytes(msg.data[0:4],'little')
        drive0=int(msg.data[4])
        drive1=int(msg.data[5])
        drive2=int(msg.data[6])
        drive3=int(msg.data[7])

        for i in range(N_channels):
            if(logging_dict['index'].values[i]==msg.arbitration_id):
                BattCelldata[i]['LDCStatus']=status
            
            if(logging_dict['Temperature msg ID'].values[i]==msg.arbitration_id-1):
                if(logging_dict['Can Sub'].values[i] ==0):
                    BattCelldata[i]['DriveCurrent']=drive0
                elif(logging_dict['Can Sub'].values[i] ==1):
                    BattCelldata[i]['DriveCurrent']=drive1
                elif(logging_dict['Can Sub'].values[i] ==2):
                    BattCelldata[i]['DriveCurrent']=drive2
                elif(logging_dict['Can Sub'].values[i] ==3):
                    BattCelldata[i]['DriveCurrent']=drive3
                elif(logging_dict['Ref index'].values[i] == msg.arbitration_id):
                    BattCelldata[i]['DriveCurrentRef']=drive3
                
    elif(msg.arbitration_id % 16==6):
        status0=int.from_bytes(msg.data[0:2],'little')
        status1=int.from_bytes(msg.data[2:4],'little')
        status2=int.from_bytes(msg.data[4:6],'little')
        status3=int.from_bytes(msg.data[6:8],'little') 
        
        for i in range(N_channels):
            if(logging_dict['Temperature msg ID'].values[i]==msg.arbitration_id-2):
                if(logging_dict['Can Sub'].values[i] ==0):
                    BattCelldata[i]['LDC status']=status0
                    BattCelldata[i]['Newdata']=1
                elif(logging_dict['Can Sub'].values[i] ==1):
                    BattCelldata[i]['LDC status']=status1
                    BattCelldata[i]['Newdata']=1
                elif(logging_dict['Can Sub'].values[i] ==2):
                    BattCelldata[i]['LDC status']=status2
                    BattCelldata[i]['Newdata']=1
                elif(logging_dict['Can Sub'].values[i] ==3):
                    BattCelldata[i]['LDC status']=status3
                    BattCelldata[i]['Newdata']=1                        
                elif(logging_dict['Ref index'].values[i] == msg.arbitration_id):
                    BattCelldata[i]['REF status']=status3
            

    elif(msg.arbitration_id % 16==8):
        NAH0=int.from_bytes(msg.data[0:2],'little')
        NAH1=int.from_bytes(msg.data[2:4],'little')
        NAH2=int.from_bytes(msg.data[4:6],'little')
        NAH3=int.from_bytes(msg.data[6:8],'little') 
        if NAH0>0:
            print("Amplitude High error CH0, ID 0x{:x} \t N:{:d}".format(msg.arbitration_id,NAH0) ) 
        if NAH1>0:
            print("Amplitude High error CH1, ID 0x{:x} \t N:{:d}".format(msg.arbitration_id,NAH1) )
        if NAH2>0:
            print("Amplitude High error CH2, ID 0x{:x} \t N:{:d}".format(msg.arbitration_id,NAH2) )
        if NAH3>0:
            print("Amplitude High error CH3, ID 0x{:x} \t N:{:d}".format(msg.arbitration_id,NAH3) )
        for i in range(N_channels):
            if(logging_dict['Temperature msg ID'].values[i]==msg.arbitration_id-4):
                if(logging_dict['Can Sub'].values[i] ==0):
                    BattCelldata[i]['NAHwarn']=NAH0
                elif(logging_dict['Can Sub'].values[i] ==1):
                    BattCelldata[i]['NAHwarn']=NAH1
                elif(logging_dict['Can Sub'].values[i] ==2):
                    BattCelldata[i]['NAHwarn']=NAH2
                elif(logging_dict['Can Sub'].values[i] ==3):
                    BattCelldata[i]['NAHwarn']=NAH3
                elif(logging_dict['Ref index'].values[i] == msg.arbitration_id):
                    BattCelldata[i]['NAHwarnRef']=NAH3
                    
# 'NAH_warn'
        #         :0,'REF status'
       # if msg.arbitration_id % 16==3 and flag_standalone==0: # decide which reference coil this should go to.

                
    # if msg.arbitration_id==base_address+3:
    #     print('\t Coil3: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))
    # if msg.arbitration_id==base_address:
    #     print('\t Coil0: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
    # if msg.arbitration_id==base_address+1:
    #     print('\t Coil1: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
    # if msg.arbitration_id==base_address+2:
    #     print('\t Coil2: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
    # if msg.arbitration_id==base_address+4:
    #     print('\t Temperature: {}  \t RH: {} '.format((int.from_bytes(msg.data[0:2],'little')/65536.0)*165.0-40.0,int.from_bytes(msg.data[2:4],'little')/65536.0*100.0))    
    # if msg.arbitration_id==base_address+5:
    #     print('\t Sensor Status: {}  '.format(int.from_bytes(msg.data[0:4],'little') ))    
    # if msg.arbitration_id==base_address+6:
    #     print('\t Sensor Status 0: {} \t Sensor Status 1: {} \t Sensor Status 2: {} \t Sensor Status 3: {} '.format(int.from_bytes(msg.data[0:2],'little'),int.from_bytes(msg.data[2:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little') ))    

    #return parsed_data



def print_message(msg: can.Message) -> None:
    """Regular callback function. Can also be a coroutine."""
    if(msg.arbitration_id % 16==8):
        print(msg)
#     base_address=0x30    
#    # msgs, error_count = ics.get_messages(device)

#     if msg.arbitration_id==base_address+3:
#         print('\t Coil3: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))
#     if msg.arbitration_id==base_address:
#         print('\t Coil0: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
#     if msg.arbitration_id==base_address+1:
#         print('\t Coil1: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
#     if msg.arbitration_id==base_address+2:
#         print('\t Coil2: {}  \t stdev: {} \t Samples: {}'.format(int.from_bytes(msg.data[0:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little')))    
#     if msg.arbitration_id==base_address+4:
#         print('\t Temperature: {}  \t RH: {} '.format((int.from_bytes(msg.data[0:2],'little')/65536.0)*165.0-40.0,int.from_bytes(msg.data[2:4],'little')/65536.0*100.0))    
#     if msg.arbitration_id==base_address+5:
#         print('\t Sensor Status: {}  '.format(int.from_bytes(msg.data[0:4],'little') ))    
#     if msg.arbitration_id==base_address+6:
#         print('\t Sensor Status 0: {} \t Sensor Status 1: {} \t Sensor Status 2: {} \t Sensor Status 3: {} '.format(int.from_bytes(msg.data[0:2],'little'),int.from_bytes(msg.data[2:4],'little'),int.from_bytes(msg.data[4:6],'little'),int.from_bytes(msg.data[6:8],'little') ))    

async def logdata() -> None:
# open the log build script
# todo : add code to start and stop indivitual channels.
    # open files. 'Filename':0,'Start_Time':0,'Newdata':0,'Filepath':0
    Headerlist=['Datapoint Number','Test Time','Current','Potential','Timestamp','LDC SENSOR','LDC REF','Ambient Temperature','Ambient RH','LDC N','LDC STD','REF N','REF STD','LDC scaled','LDC status','REF status','DriveCurrent','DriveCurrentRef','NAHwarn','NAHwarnRef']
    SubHeader=['none','second','amp','volt','epoch','none','none','celsius','percent','none','none','none','none','none','none','none','none','none','none','none']
    while True:
        await asyncio.sleep(.1)
        
        for i in range(N_channels):
            #print('Battery :'+ str(i) + '\t Log status: ' + BattCelldata[i]['LogStatus'])
            
            if BattCelldata[i]['LogStatus']=='Start' :
                #BattCelldata[i]['Filename']= File_Prefix+logging_dict['PROJECT'].values[i]+'_CELL{:03d}'.format(logging_dict['Cell #'].values[i])+'_CH{:03d}'.format(logging_dict.index[i])+'.vdf' # add date,
                #df=pd.DataFrame([{k:v for k,v in BattCelldata[i].items()if k in Headerlist}])
                #HeaderInfo = ['Start Time: {0}\r\nTimezone: America/New_York\r\nChannel Number: {1}\r\n[DATA START]'.format(int(time.time()*1000),i+1)]
                with open(BattCelldata[i]['Filename'], 'w',newline='') as file: # use newline='' to fix bug on windows
                    headerwriter=csv.writer(file,delimiter="\t", quotechar='\"', quoting=csv.QUOTE_NONE,escapechar='\t')
                    BattCelldata[i]['StartTime']=int(time.time()*1000)
                    BattCelldata[i]['Timestamp']=BattCelldata[i]['StartTime']
                    headerwriter.writerow(['Start Time: {0}'.format(int(BattCelldata[i]['StartTime']))])
                    
                    headerwriter.writerow(['Format Version: 3.0'])
                    headerwriter.writerow(['Timezone: America/New_York'])
                    headerwriter.writerow(['Channel Number: {0}'.format(int(i+1))])
                    headerwriter.writerow(['[DATA START]'])
                    headerwriter.writerow(Headerlist)
                    headerwriter.writerow(SubHeader)
                    BattCelldata[i]['LogStatus']='Running'  
            #if(BattCelldata[i]['LogStatus']=='Closed'):
                #do nothing
            elif BattCelldata[i]['LogStatus']=='Running' :


                if(BattCelldata[i]['Newdata']==1):
                    BattCelldata[i]['Newdata']=0
                    if(BattCelldata[i]['Datapoint Number']>0): #skip data points.
                        df=pd.DataFrame([{k:v for k,v in BattCelldata[i].items()if k in Headerlist}])
                        try:
                            df.to_csv(BattCelldata[i]['Filename'], mode='a', index=False, header=False, sep ='\t')
                        except Exception as x:
                            print x
        # Log some new data



async def main() -> None:
    """The main function that runs in the loop."""

       # BattCelldata[i]['Filepath']= open('C:\\VDFData\\'+BattCelldata[i]['Filename']+'.vdf')

   # bus = can.Bus(bustype="gs_usb", channel=dev.product,  index=0, bitrate=250000)
    with can.Bus(  # type: ignore
 #    bustype="gs_usb",  channel=dev.product, bus=dev.bus, address=dev.address, index=0,bitrate=500000
      interface="neovi", channel=1, receive_own_messages=False,bitrate=500000
#        interface="socketcan", channel=1, receive_own_messages=False,bitrate=500000
    ) as bus:
        #reader = can.AsyncBufferedReader()
        logger = can.Logger(File_Prefix+'logfile'+datetime.now().strftime("%Y%m%d_%H_%M_%S")+'.asc')

        listeners: List[MessageRecipient] = [
           # print_message,  # Callback function
            parse_msg,
            #reader,  # AsyncBufferedReader() listener
            logger,  # Regular Listener object
        ]

        def close_future(future, loop):
            loop.call_later(10, future.cancel)
            future.cancel()

        loop = asyncio.get_event_loop()
        future = asyncio.Future()

        app = QApplication.instance()
        if hasattr(app, "aboutToQuit"):
            getattr(app, "aboutToQuit").connect(
                functools.partial(close_future, future, loop)
            )
   #initailize all the can interfaces to the last known good value.
        for canidme in [0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0x100,0x110,0x120,0x210,0x220,0x230,0x240,0x250,0x260,0x270,0x280,0x290,0x300,0x310,0x320,0x410,0x420,0x430,0x440,0x450,0x460,0x470,0x480,0x490,0x500,0x510,0x520]:
            dfs = dfdc.loc[dfdc['can_id'] == canidme]
            dfs = dfs.reset_index()
            # self.dc_ch_idx[0]=dfs.dc_0[0]
            # self.dc_ch_idx[1]=dfs.dc_1[0]
            # self.dc_ch_idx[2]=dfs.dc_2[0]
            # self.dc_ch_idx[3]=dfs.dc_3[0]
            candata=[int(dfs.dc_0[0]),  int(dfs.dc_1[0]), int(dfs.dc_2[0]), int(dfs.dc_3[0]),0, 0, 0, 0 ]
            # Reset the drive current
            msg = can.Message(arbitration_id=int(canidme+0xA),data=candata , is_extended_id=False)

            
            try:
                bus.send(msg)
                print(f"Message sent on {bus.channel_info}")
                time.sleep(.05)
            except can.CanError:
                print("Message NOT sent")


        mainWindow = MainWindow(bus)
        mainWindow.show()

        timer = QtCore.QTimer()
        timer.timeout.connect(mainWindow.sens_val)
        timer.start(500)  # every 10,000 milliseconds
        
        asyncio.ensure_future(logdata())
        
        # Create Notifier with an explicit loop to use for scheduling of callbacks
        loop = asyncio.get_running_loop()
        notifier = can.Notifier(bus, listeners, loop=loop)

        # Start sending first message
        #bus.send(can.Message(arbitration_id=0))
        await future
        # print("Bouncing 10 messages...")
        # for _ in range(10):
        #     # Wait for next message from AsyncBufferedReader
            
        #     # Delay response
        #     await asyncio.sleep(0.5)
        #     msg.arbitration_id += 1
        #     bus.send(msg)

        # # Wait for last message to arrive
        # await reader.get_message()
        print("stopping!")
        #while True:
            #msg = await reader.get_message()

        # Clean-up
        notifier.stop()
      #  for i in range(N_channels):
      #      close(BattCelldata[i]['Filepath'])
        return True

if __name__ == "__main__":
#    asyncio.run(main())
    try:
        qasync.run(main())
    except asyncio.exceptions.CancelledError:
        sys.exit(0)


#devices = ics.find_devices()
#for device in devices:
#     print(device.Name, device.SerialNumber)
#ics.open_device(devices[0])
#device=devices[0]

#while True:
#    receive_can(device)
    