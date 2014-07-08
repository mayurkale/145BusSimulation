#Help : python read.py M.mat M

import sys
import socket

import scipy as sci
import scipy.linalg

import scipy.io as sio
from cmath import rect
from cmath import polar

from numpy import array
from numpy import *


M = sio.loadmat(sys.argv[1])
M = M[sys.argv[2]]

Minv = linalg.pinv(M)

f = open(sys.argv[2]+"MMpinv.txt","w")
for i in Minv:
	for j in i:
		f.write(str(j.real)+' '+str(j.imag)+'\n')

f.close()


print Minv.shape
#count = 0
#for i in Minv:
#	for j in i:
#		count +=1
#		if (abs(j.real) < 0.000000001) & (abs(j.imag) < 0.000000001):
#			continue
#		else:
#			count +=1
#			print str(j.real)+' '+str(j.imag)
#print count


