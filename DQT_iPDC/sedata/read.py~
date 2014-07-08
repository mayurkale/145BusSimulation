#Help : python read.py Mer.mat Mer
#	   python read.py Mwr.mat Mwr
#	   python read.py Msuper.mat Msuper

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

MH = matrix(M)
MH = MH.H
MMHinv = linalg.inv(MH*M)
MMHinvdiag = array(MMHinv.diagonal().T)

P = M * MMHinv * MH
Pdiag = array(P.diagonal().T)

print M.shape
print Pdiag.shape
print MMHinvdiag.shape

#f = open(sys.argv[2]+"M.txt","w")
#for i in M:
#	for j in i:
#		f.write(str(j.real)+' '+str(j.imag)+'\n')

#f.close()

#f = open(sys.argv[2]+"Mpinv.txt","w")
#for i in Minv:
#	for j in i:
#		f.write(str(j.real)+' '+str(j.imag)+'\n')

#f.close()

#f = open(sys.argv[2]+"P.txt","w")
#for j in Pdiag:
#	f.write(str(j[0].real)+' '+str(j[0].imag)+'\n')

#f.close()

#f = open(sys.argv[2]+"MHMinv.txt","w")
#for j in MMHinvdiag:
#	f.write(str(j[0].real)+' '+str(j[0].imag)+'\n')

#f.close()


#print Minv.shape
#count = 0
#for i in Minv:
#	for j in i:
##		count +=1
#		if (abs(j.real) < 0.000000001) & (abs(j.imag) < 0.000000001):
#			continue
#		else:
#			count +=1
##			print str(j.real)+' '+str(j.imag)
#print count

#print M.shape
#count = 0
#for i in M:
#	for j in i:
##		count +=1
#		if (abs(j.real) < 0.000000001) & (abs(j.imag) < 0.000000001):
#			continue
#		else:
#			count +=1
##			print str(j.real)+' '+str(j.imag)
#print count

#print M.shape
#count = 0
#for i in M:
#	for j in i:
##		count +=1
#		if (j.real == 0.0) & (j.imag == 0.0):
#			continue
#		else:
#			count +=1
##			print str(j.real)+' '+str(j.imag)
#print count
