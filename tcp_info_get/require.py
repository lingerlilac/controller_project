import socket
import datetime
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('192.168.11.108', 12307))
while True:
    filedata = datetime.datetime.now()
    filedata = str(filedata) + ','
    # for i in range(0, 10):
    #     filedata = filedata + ',' + filedata
    if not filedata:
        break
    # print len(filedata)
    s.send(filedata)
print 'send over...'
