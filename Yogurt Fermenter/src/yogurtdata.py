import datetime
import os
import socket
import time

import numpy as np
import pandas as pd

from matplotlib import pyplot as plt, style
import matplotlib.dates as md

from src.PIDProgram import PIDProgram


class YogourtFermenter():
    def __init__(self):

        self.temperatureevents = []
        self.tempbysec = []
        self.CV = []
        self.SPlist = []
        self.SP = 33
        self.overridepid = None
        self.currentSP = self.SP
        self.currentCV = 0
        self.currenttemp = 0
        self.currentoutput = 0
        self.starttime = datetime.datetime.now()
        self.setPidvalues()
        # Create a UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.esp8266_ip = "192.168.2.190"
        self.esp8266_port = 5000
        # Set the IP address and port to listen on
        server_address = ('', self.esp8266_port)

        self.sock.bind(server_address)
        print('Listening on {}:{}'.format(*server_address))
        self.creategraph()
        self.pidprogram = PIDProgram(self)
        self.listeningloop()


    def setPidvalues(self):
        #float aggKp = 3.27, aggKi = 0.0008, aggKd = 646.15;
        multiplier = 2.56
        self.aggKp = 3.27 * multiplier
        self.aggKi = 0.0008 * multiplier
        self.aggKd = 646.15 * multiplier
        # consKp = 1.91, consKi = 0.0004, consKd = 6.282;
        self.consKp = 3 * multiplier
        #self.consKi = 0.0004 * multiplier
        self.consKi = 0.001 * multiplier # 0.035
        self.consKd = 6.282 * multiplier

    def creategraph(self):
        if plt.get_fignums():
            print("Graphic already opened")
            return
        self.graphstarttime = time.time()
        style.use('fivethirtyeight')

        self.fig = plt.figure()
        #self.fig.canvas.mpl_connect('close_event', self.pltwindowclosed)
        self.ax1 = self.fig.add_subplot(1, 1, 1)
        plt.ion()
        # plt.show()

        xfmt = md.DateFormatter('%Hh%Mm')
        self.ax1.xaxis.set_major_formatter(xfmt)
        self.line1 = self.ax1.plot([], [])  # add an empty line to the plot
        self.line2 = self.ax1.plot([], [])
        self.fig.show()  # show the window (figure will be in foreground, but the user may move it to background)


    def animate(self):
        if not plt.get_fignums():
            print("No graph")
            return
        if self.temperatureevents == []:
            return
        # self.ax1.clear()
        # self.ax1.plot([i[0] for i in self.temperatureevents], [i[1] for i in self.temperatureevents])
        firsttimeevent = self.temperatureevents[0][0]
        x, y = [datetime.datetime.fromtimestamp((i[0] - firsttimeevent).total_seconds()) for i in self.temperatureevents], [
            i[1] for i in self.temperatureevents]
        #print("firsttimeevent", firsttimeevent)
        #print(self.temperatureevents, "self.temperatureevents")
        #print(x, y)
        #self.line1.set_data(x, y)
        x = []
        for i in range(len(self.tempbysec)):
            x.append(self.starttime + datetime.timedelta(seconds=i-1))

        self.ax1.lines[0].set_data(x, self.tempbysec)  # Temperature
        self.ax1.lines[1].set_data(x, self.CV)  # PID PWM
        self.ax1.lines[0].set_zorder(2)
        self.ax1.relim()  # recompute the data limits
        self.ax1.autoscale_view()  # automatic axis scaling
        self.fig.canvas.flush_events()

    def appendnewtemperatureevent(self,message):
        message = message.replace("Sensor temp: ","")
        print()
        # temperature = float(data['StatusSNS']['MAX6675']['Temperature'])
        # mesuretime = data['StatusSNS']['Time']
        # print(mesuretime)
        # mesuretime = parser.parse(mesuretime)
        # print(mesuretime)
        self.currenttemp = float(message)
        self.temperatureevents.append([datetime.datetime.now(), float(message)])
        #print(self.temperatureevents)
        self.animate()

    def line_prepender(self,filename, line):
        with open(filename, 'r+') as f:
            content = f.read()
            f.seek(0, 0)
            f.write(line.rstrip('\r\n') + '\n' + content)
    def savetempbysectocsv(self):
        array = np.zeros((3,len(self.tempbysec)))
        array[0] = self.tempbysec
        array[1] = self.CV
        array[2] = self.SPlist
        filepath = os.getcwd()+"/yogurtpid2.csv"
        np.savetxt(filepath,
                   array.T,
                   delimiter=";",
                   fmt='% s')
        self.line_prepender(filepath,"PV;CV;SP")

        # tempbysec_dict = {'PV': self.tempbysec, 'CV': self.CV,'SP' : [str(self.SP).replace(',','.') for i in self.CV]}
        # tempbysec_df = pd.DataFrame(tempbysec_dict)
        # tempbysec_df.to_csv(os.getcwd()+"/yogurtpid.csv")
    def sendMessage(self,message):
        self.sock.sendto(message.encode(), (self.esp8266_ip, self.esp8266_port))
    def setaggPID(self):
        self.sendMessage("SetTuningsagg("+str(self.aggKp)+","+str(self.aggKi)+","+str(self.aggKd) + ")")
    def setconsPID(self):
        self.sendMessage("SetTuningscons("+str(self.consKp)+","+str(self.consKi)+","+str(self.consKd) + ")")
    def setSP(self, SP):
        print("Setting new temperature target at :",SP)
        self.SP = SP
        self.sendMessage("SetSP(" + str(self.SP) + ")")
    def setManualPIDTuning(self):
        self.sendMessage("ExternalPIDTest")
    def setPIDOverride(self):
        if self.overridepid:
            print("Overriding PID")
            self.sendMessage("OverridePID")
        else:
            print("Stop PID Override and perform pid test")
            self.sendMessage("EnablePIDandTest")
    def listeningloop(self):
        lastsec = int(time.time())
        print(lastsec)
        count = 0
        while True:
            if time.time() > lastsec + 1:
                lastsec = int(time.time())
                self.tempbysec.append(self.currenttemp)
                self.CV.append(self.currentCV)
                self.SPlist.append(self.currentSP)
                #print("self.tempbysec",self.tempbysec)
                #self.setaggPID()
                #self.setconsPID()
                #self.setSP(33)
                #self.setManualPIDTuning()
                count += 1
            self.pidprogram.applyProgram()
            data, address = self.sock.recvfrom(4096)
            message = data.decode()
            print(message)
            print(time.time())
            if count > 5:
                count = 0
                self.savetempbysectocsv()
            if "SetPoint:" in message:
                self.currentSP = float(message.replace("SetPoint:",""))
                if not self.currentSP == self.SP:
                    self.setSP(self.SP)
                    print("SP was not set !! Resetting it ....")
                continue
            if "Override PID:" in message:
                message = message.replace("Override PID: ","")
                if "0" in message and self.overridepid or "1" in message and not self.overridepid:
                    self.setPIDOverride()
                    print("PID Override not set. Setting PID Overide to ",self.overridepid )
                continue
            if "Sensor temp:" in message:
                self.appendnewtemperatureevent(message)
                continue
            if "Output:" in message:
                message = message.replace("Output:","")
                if "nan" in message:
                    self.currentCV = 0
                    continue
                self.currentCV = int((float(message)/255)*100)
                #print("self.currentCV",self.currentCV)
                continue
            if "PIDsettingsagg(Kp,Ki,Kd):" in message:
                message = message.replace("PIDsettingsagg(Kp,Ki,Kd):", "")









yogourtfermenter = YogourtFermenter()