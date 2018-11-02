import socket
import datetime
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('192.168.11.106', 12307))
while True:
    filedata = datetime.datetime.now()
    filedata = str(filedata) + ','
    # for i in range(0, 10):
    #     filedata = filedata + ',' + filedata
    if not filedata:
        break
    # print len(filedata)
    time.sleep(1)
    s.send(filedata)
print 'send over...'
