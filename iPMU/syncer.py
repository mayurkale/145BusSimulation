# Echo client program
# Help: python syncer.py 127.0.0.1 13000 127.0.0.1 13001

import socket
import sys
import time
import datetime

HOST1 = sys.argv[1]    
PORT1 = int(sys.argv[2])

HOST2 = sys.argv[3]   
PORT2 = int(sys.argv[4])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
q = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((HOST1, PORT1))
q.connect((HOST2, PORT2))

t = datetime.datetime.today()
future = datetime.datetime(t.year,t.month,t.day,t.hour,t.minute,t.second+2)

time.sleep((future-t).seconds + (future-t).microseconds / 1000000.0)

s.sendall('')
q.sendall('')

s.close()
q.close()

