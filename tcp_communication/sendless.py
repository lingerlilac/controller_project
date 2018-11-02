import socket
import datetime
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('192.168.1.14', 12308))
s.connect(('192.168.1.17', 12307))
while True:
    filedata = datetime.datetime.now()
    filedata = str(filedata)
    for i in range(0, 5):
    	filedata += str(filedata) + ','
    # for i in range(0, 10):
    #     filedata = filedata + ',' + filedata
    if not filedata:
        break
    # print len(filedata)
    s.send(filedata)
print 'send over...'
