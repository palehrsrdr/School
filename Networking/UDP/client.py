import sys
from tkinter import *
from tkinter.simpledialog import *
import threading
import socket
import time
import os
class Listener(threading.Thread):
	def __init__(self,sock,callback):
		threading.Thread.__init__(self)
		self.sock=sock
		self.callback=callback
		self.sock.bind(( "0.0.0.0", 0))
		connection(self.sock)
	def run(self):

		while 1:
			data,addr = self.sock.recvfrom(65400)
			data = data.decode("utf-8")
			print("Got data from",addr,":",data)
			root.after(1,self.callback,data,addr)
	def stop(self):
		os._exit(1)
def send(*junk):
	txt = textline.get()
	textline.delete(0,len(txt))
	message = "CHAT:" + myname + ":" + txt
	message = message.encode("utf-8")
	sock.sendto(message, (server, 31337))


def recv(d,a):
	#parse the message, decide what to do with it.
	#make sure it is from the server (same IP address we entered)
	print(d,"\n")
	msg,name,txt = d.split(":",2)
	#if it is a chat msg, or error display it
	if msg == "CHAT" or msg == "ERROR":

		nowtime = str(time.asctime()).split(" ")[3] + " "

		textbox.insert(END, nowtime)
		if name == myname:
			textbox.insert(END,"<"+name+">","highlight")

		else:
			textbox.insert(END,"<"+name+">", "Pinkify")

		textbox.insert(END," "+txt)
		textbox.insert(END,"\n")
		textbox.see(END)
	#if it is a ping message, send pong message.
	elif msg == "PING":
		pong()
	#elif msg == "ID":
	else:
		return


def connection(sock):
	#connection to the chat server.
	message = "CONNECT:" + myname + ":" + "0"
	message = message.encode("utf-8")

	sock.sendto(message, (server, 31337))

def quit_program():
	#send disconnect message to the server.
	message = "DISCONNECT:" + myname + ":" + "0"
	message = message.encode("utf-8")
	sock.sendto(message,  (server, 31337))
	root.destroy()
	thr.stop()

def pong():
	#Send the server the pong message to stay alive.
	message = "PONG:" + myname + ":0"
	message = message.encode("utf-8")
	sock.sendto(message, ( server, 31337))

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
root=Tk()
root.withdraw()

#ask for user's name
myname=askstring("Hello","What is your name?")
#server=askstring("Hello","What is the server's IP address?")

#myname = "wat"
server = "127.0.0.1"    #coment this out and uncoment 2 lines above this for
						#custom ip adress

#make listener thread
thr = Listener(sock,recv)
thr.start()
root.title("User: " + myname + " Connected To: " + server)
#show root window; put some widgets in it
root.deiconify()
fr = Frame(root)
sbar = Scrollbar(fr,orient=VERTICAL)
textbox = Text(fr,yscrollcommand=sbar.set, wrap=WORD)
textbox.tag_config("highlight",background="yellow")
textbox.tag_config("Pinkify", background = "pink")
textbox.tag_config("normal",background="white")

sbar.config(command=textbox.yview)
sbar.pack(side=RIGHT,expand=NO,fill=Y)
textbox.pack(side=LEFT,expand=YES,fill=BOTH)
fr2 = Frame(root)
textline = Entry(fr2)
textline.bind("<Return>",send)
sendbutton = Button(fr2,text="Send",command=send )
sendbutton.pack(side=RIGHT,expand=NO)
textline.pack(side=LEFT,expand=YES,fill=X)
fr2.pack(side=BOTTOM,expand=NO,fill=X)
fr.pack(side=TOP,expand=YES,fill=BOTH)
root.protocol("WM_DELETE_WINDOW",quit_program)
root.mainloop()
