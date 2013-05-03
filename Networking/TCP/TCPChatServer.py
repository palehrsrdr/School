## Clinton Sexton

import sys
from tkinter import *
from tkinter.simpledialog import *
from datetime import datetime
from datetime import timedelta
import socket
import time
import random
import threading
import select
cc = []
#list of connected clients: [name, ip, timeoutCounter]
ClientList = []

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

sock.bind(("0.0.0.0", 31337))

sock.listen(5)


def send(message):
	totalBytes = len(message)
	totalBytes += 1
	words = ("%05d"%totalBytes + ":" + message).encode("utf-8")

	if len(cc) > 0:
		for client in cc:
			if not client._closed:

				client.send(words)
	else:
		pass
def recieve(x):

	newSocket = x

	msg =""

	c, a = newSocket.recvfrom(5)
	c = c.decode("UTF-8")
	while int(c) > 0:
		tmp,a = newSocket.recvfrom(int(c))
		tmp = tmp.decode("UTF-8")
		msg += tmp
		c = int(c) - len(tmp)
	trash,msg = msg.split(":",1)

	opcode, name, message = msg.split(":", 2)

	if not contains(name, newSocket):
		if opcode == "CONNECT":
			connection(name, newSocket)
			return
		else:
			sendError(name, newSocket, 2)
			return
	if reallyContains(name, newSocket):
		if opcode == "CONNECT":
			sendError(name, newSocket, 2)
			disconnect(name,newSocket)
			return
		elif opcode == "PONG":
			pongRcvd(name)
			return
		elif opcode == "DISCONNECT":
			disconnect(name,newSocket)
			return
		elif opcode == "CHAT":
			txtMsg(name, message)
			return
		else:
			sendError(name, newSocket, 1)
			return
	sendError(name, newSocket, 2)
	sendError(name, newSocket, 1)
	return

def addTime():
	timeAtCall = time.clock()
	x=1
	while x==1:
		timeElapsed =  (time.clock() - timeAtCall)
		for user in ClientList:
			user[2] += timeElapsed
			if user[2] >= 10 and user[2] < 25 and user[4] == "n":
				print("Dangerously close to timeout: ", user[0])
				user[4] = "y"
				sendPing(user[0], user[1])
			elif user[2] > 12:
				print("timeout exceeded on: ", user[0])
				x = 0
				disconnect(user[0], user[1])

		timeAtCall = time.clock()
		time.sleep(0.1)

def contains(name, src):
	for i in range(len(ClientList)):
		if name.lower() == ClientList[i][0].lower():
			return True
	return False

def reallyContains(name, src):
	for i in range(len(ClientList)):
		if name.lower() == ClientList[i][0].lower():
			if  ClientList[i][1] == src:
				return True
	return False

def connection(name, ip):
	userid = random.randint(1,5000)
	ClientList.append([name, ip, 0.0, userid, "n"])
	message = "CHAT" + ":" + name + ":" + "Has connected!"
	send(message)

def disconnect(name, src):
	message = "CHAT" + ":" + name + ":" + "Has disconnected!"
	for i in range(len(ClientList)-1):
		if ClientList[i][0].lower() == name.lower() and  ClientList[i][1] == src:
			ClientList.pop(i)

	src.close()

	send(message)

def sendPing(user, src):
	message = "PING:0:0"

	send(message)

def pongRcvd(name):
	for client in ClientList:
		if client[0] == name:
			print("Pong recieved from ", client[0])
			client[2] = 0
			client[4] = "n"

def txtMsg(name,words):
	for client in ClientList:
		if client[0] == name:
			client[2] = 0
	message = "CHAT" + ":" + name +":" + words
	send(message)

def sendError(name, src, number):
	if number == 1:
		message = "ERROR:"+name+":You must reconnect to the server.  Restart your ClientList to do so"
	else:
		message = "ERROR:"+name+":Duplicate username detected.  Connection Denied!"
	totalBytes = len(message)
	totalBytes += 1
	words = ("%05d"%totalBytes + ":" + message).encode("utf-8")
	src.send(words)

to = threading.Thread(target=addTime)
to.daemon=True
to.start()

print("Serving....")

#main loop to run forever!
while 1:
	try:
		rr, rw, rx = select.select([sock]+cc, [], [])
	except ValueError:
		for x in cc:
			if x._closed:
					cc.remove(x)
	for x in rr:
		if x == sock and not x._closed:
			tmp, a = sock.accept()
			cc.append(tmp)
		else:
			if x._closed:
				continue
			else:
				recieve(x)