## Clinton Sexton

import sys
from tkinter import *
from tkinter.simpledialog import *
import threading
import socket
import time
import select
import os

sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

root=Tk()
root.withdraw()

def send(*junk):
    txt = textline.get()
    textline.delete(0,len(txt))
    message = "CHAT:" + myname + ":" + txt
    totalBytes = len(message)
    totalBytes += 1
    words = ("%05d"%totalBytes + ":" + message).encode("utf-8")
    sock.send(words)

def gotMail(msg):

    print("Checking...")
    opcode, name, string = msg.split(":", 2)
    if opcode == "CHAT" or opcode == "ERROR":
        makeText(name, string)
    elif opcode == "PING":
        pong()

    else:
        return

def makeText(name, string):
    nowtime = time.asctime().split(" ")[4] + " "
    textbox.insert(END, nowtime)
    if name == myname:
        textbox.insert(END,"<"+name+">","highlight")
    else:
        textbox.insert(END,"<"+name+">", "Pinkify")
    textbox.insert(END,(" " + string))
    textbox.insert(END,"\n")
    textbox.see(END)

def connection():
    sock.bind(("0.0.0.0", 0))
    sock.connect((server, 31337))
    message = "CONNECT:" + myname + ":" + "0"
    totalBytes = len(message)
    totalBytes += 1
    words = ("%05d"%totalBytes + ":" + message).encode("utf-8")
    sock.send(words)

def quit_program():
    message = "DISCONNECT:" + myname + ":" + "0"
    totalBytes = len(message)
    totalBytes += 1
    words = ("%05d"%totalBytes + ":" + message).encode("utf-8")
    sock.send(words)
    root.destroy()
    thr.stop()

def pong():
    message = "PONG:" + myname + ":0"
    totalBytes = len(message)
    totalBytes += 1
    words = ("%05d"%totalBytes + ":" + message).encode("utf-8")
    sock.send(words)

class Listener(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        connection()
    def run(self):
        while 1:
            msg = ""
            c, a = sock.recvfrom(5)
            c = c.decode("UTF-8")
            print("recieved!")
            while int(c) > 0:
                tmp, a = sock.recvfrom(int(c))
                tmp = tmp.decode("UTF-8")
                msg += tmp
                c = int(c) - len(tmp)
            trash,msg = msg.split(":",1)
            print(msg)
            root.after(1, gotMail, msg)
    def stop(self):
        os._exit(1)

myname=askstring("Hello","What is your name?")
server = "localhost"


root.title("User: " + myname + " Connected To: " + server)
#show root window; put some widgets in it
root.deiconify()
fr = Frame(root)
sbar = Scrollbar(fr,orient=VERTICAL)
textbox = Text(fr,yscrollcommand=sbar.set, wrap=WORD)
textbox.tag_config("highlight",background="yellow")
textbox.tag_config("Pinkify", background = "pink")
textbox.tag_config("Left", background = "red")
textbox.tag_config("Joined", background = "green")
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
thr=Listener()
thr.start()
root.mainloop()
