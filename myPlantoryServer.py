#!/usr/bin/env python
 
import socket
import time

# TCP_IP = ''
TCP_PORT = 8081
BUFFER_SIZE = 1024  # Normally 1024, but we want fast response
serverAddress = ('localhost', TCP_PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', TCP_PORT))
s.listen(1)

print 'starting up on %s port %s' % serverAddress

print "Escuchando en puerto: %d" % (TCP_PORT)

while 1:
    try:
        conn, addr = s.accept()
        print 'Connection address:',addr
        time.sleep(1)
        data = conn.recv(BUFFER_SIZE)
        if not data: 
            break
        print "received data:", data
        data = "[dataRX]: " + data 
        #conn.send(data)  # echo
        conn.close()
    except   KeyboardInterrupt:  
        s.close()
        print "Server terminado manualmente"
        break