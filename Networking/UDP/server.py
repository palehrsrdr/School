import sys
from tkinter import *
from tkinter.simpledialog import *
from datetime import datetime
from datetime import timedelta
import socket
import time
import random
import threading

uList = []
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 31337))

def send(message):
	words = message.encode()
	for u in uList:
		sock.sendto(words, (u[1]))

def recv():
	msg, src = sock.recvfrom(65535)
	msg = msg.decode()
	print(msg)
	action, name, message = msg.split(":", 2)
	if not contains(name, src):
		if action == "CONNECT":
			connection(name, src)
			return
		else:
			sendError(name, src, 2)
			return
	if reallyContains(name, src):
		if action == "CONNECT":
			sendError(name, src, 2)
			disconnect(name,src)
			return
		elif action == "PONG":
			pongRcvd(name)
			return
		elif action == "DISCONNECT":
			disconnect(name,src)
			return
		elif action == "CHAT":
			txtMsg(name, message)
			return
		else:
			sendError(name, src, 1)
			return
	sendError(name, src, 2)
	sendError(name, src, 1)
	return

def addTime():
	timeAtCall = time.clock()
	while True:
		timeElapsed =  (time.clock() - timeAtCall)
		for u in uList:
			u[2] += timeElapsed
			if u[2] >= 10 and u[2] < 25 and u[4] == "n":
				print("Dangerously close to timeout: ", u)
				u[4] = "y"
				sendPing(u[0], u[1])
			elif u[2] > 25:
				print("timeout exceeded on: " ,u)
				disconnect(u[0], u[1])
		timeAtCall = time.clock()
		time.sleep(0.1)

def contains(name, src):
	for i in range(len(uList)):
		if name.lower() == uList[i][0].lower():
			return True
	return False

def reallyContains(name, src):
	for i in range(len(uList)):
		if name.lower() == uList[i][0].lower():
			if uList[i][1][0] == src[0] and uList[i][1][1] == src[1]:
				return True
	return False

def connection(name, ip):
	userid = random.randint(1,5000)
	uList.append([name, ip, 0.0, userid, "n"])
	message = "CHAT" + ":" + name + ":" + "Has connected!"
	send(message)

def disconnect(name, src):
	message = "CHAT" + ":" + name + ":" + "Has disconnected!"
	if len(uList) > 0:
		for i in range(len(uList)-1):
			if uList[i][0].lower() == name.lower() and uList[i][1][0] == src[0] and uList[i][1][1] == src[1]:
				uList.pop(i)

	else:
		return
	send(message)

def sendPing(user, src):
	message = "PING:0:0"
	words = message.encode()
	sock.sendto(words, (src))

def pongRcvd(name):
	for u in uList:
		if u[0] == name:
			u[2] = 0
			u[4] = "n"

def txtMsg(name,words):
	for u in uList:
		if u[0] == name:
			u[2] = 0
	message = "CHAT" + ":" + name +":" + words
	send(message)

def sendError(name, src, number):
	if number == 1:
		message = "ERROR:"+name+":You must reconnect to the server.  Restart your chatp rogram."
	else:
		message = "ERROR:"+name+":Duplicate username detected.  Connection Denied!"
	message = message.encode()
	sock.sendto(message, (src))

timeoutStuff = threading.Thread(target=addTime)
timeoutStuff.daemon=True
timeoutStuff.start()

print("Serving....")
motd = "Welcome to this janky server where hardly anyone shows their face!"
print("\n\n",motd,"\n\n")

#main loop to run forever!
while 1:
    recv()
