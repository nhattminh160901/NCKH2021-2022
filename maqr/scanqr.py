import cv2
from pyzbar import pyzbar
import pyrebase
import sqlite3
from tkinter import *
from PIL import ImageTk, Image
from datetime import datetime
import csv
import serial as sr
import time

def getImage(id):
    try:
        img = Image.open("dataimage\\"+id[0]+".jpg")
        return img 
    except:
        print("No image found")
        img = None
        return img

def get_profile(id, fullName):
    try:
        conn = sqlite3.connect('data.db')
        query = "Select * from People WHERE (ID='"+str(id)+"' AND FullName='"+str(fullName)+"')"
        cursor = conn.execute(query)
        profile = None

        for row in cursor:
            profile = row
        conn.close()
        return profile
        
    except sqlite3.OperationalError:
        print("Can't be found")

def zoom_center(frame, zoom_factor=1.5):
    y_size = frame.shape[0]
    x_size = frame.shape[1]
    
    x1 = int(0.5*x_size*(1-1/zoom_factor))
    x2 = int(x_size-0.5*x_size*(1-1/zoom_factor))
    y1 = int(0.5*y_size*(1-1/zoom_factor))
    y2 = int(y_size-0.5*y_size*(1-1/zoom_factor))

    frame = frame[y1:y2,x1:x2]
    return cv2.resize(frame, None, fx=zoom_factor, fy=zoom_factor, interpolation=cv2.INTER_CUBIC)

def read_qrcode(frame, ckd, start):
    qrcodes = pyzbar.decode(frame, symbols=[pyzbar.ZBarSymbol.QRCODE])
    Id = ""
    fullName = ""
    dem = 0
    db = firebase.database()

    for qrcode in qrcodes:
        x, y , w, h = qrcode.rect
        qrcode_info = qrcode.data.decode("utf-8")
        cv2.rectangle(frame, (x, y),(x+w, y+h), (0, 255, 0), 2)
        for i in qrcode_info:
            if dem == 0:
                if i == "|":
                    dem+=1
                else:
                    Id = Id + i
            elif dem == 1:
                if i == "|":
                    dem+=1
                else:
                    fullName = fullName + i
            elif dem == 2:
                break
        information = get_profile(Id, fullName)
        if information != None:
            if information != ckd:
                temp = db.child("informationStudent").child("temp").get().val()
                heart_rate = db.child("informationStudent").child("nhip_tim").get().val()
                spo2 = db.child("informationStudent").child("spo2").get().val()
                opendoor = db.child("informationStudent").child("opendoor").get().val()
                ckd = information
                start = time.time()
                if opendoor:
                    ser.write("m".encode("ascii"))
                    
                # cv2.putText(frame, information[0], (x + 10, y+h+30), font, 1, (255, 255, 255), 1)
                # cv2.putText(frame, information[1], (x + 10, y+h+60), font, 1, (255, 255, 255), 1)
                # cv2.putText(frame, information[2], (x + 10, y+h+90), font, 1, (255, 255, 255), 1)
                
                now = datetime.now()
                dt_string = now.strftime("%H:%M:%S")
                date = now.strftime("%d_%m_%Y")
                try:
                    f = open("log\\"+information[0]+"_"+information[1]+"_"+information[2]+".csv", "r", encoding = "utf-8")
                    log = csv.reader(f)
                    newLog = []
                    for row in log:
                        newLog.append(row)
                    f.close()
                    status = "Nothing"
                except:
                    status = None

                if status is None:
                    f = open("log\\"+information[0]+"_"+information[1]+"_"+information[2]+".csv", "w", encoding = "utf-8", newline = "")
                    create = csv.writer(f)
                    create.writerow([("Date"), ("Time"), ("Temperature"), ("Heart rate"), ("SpO2")])
                    create.writerow((date, dt_string, temp, heart_rate, spo2))
                    f.close()
                else:
                    newLog.append((date, dt_string, temp, heart_rate, spo2))
                    f = open("log\\"+information[0]+"_"+information[1]+"_"+information[2]+".csv", "w", encoding = "utf-8", newline = "")
                    save = csv.writer(f)
                    save.writerows(newLog)
                    
                window=Tk()
                Label(window, text="Chứng minh nhân dân: "+information[0], fg="black", font="Helvetica").pack()
                Label(window, text="Họ và Tên: "+information[1], fg="black", font="Helvetica").pack()
                Label(window, text="Mã sinh viên: "+information[2], fg="black", font="Helvetica").pack()
                Label(window, text="Ngày tháng năm sinh: "+information[3], fg="black", font="Helvetica").pack()
                Label(window, text="Chức vụ: "+information[4], fg="black", font="Helvetica").pack()
                Label(window, text="Nhiệt độ đo được: "+str(temp)+"°C", fg="black", font="Helvetica").pack()
                Label(window, text="Nhịp tim đo được: "+str(heart_rate)+" bmp", fg="black", font="Helvetica").pack()
                Label(window, text="Nhịp tim đo được: "+str(spo2)+" %", fg="black", font="Helvetica").pack()
                img = getImage(information)
                if img != None:
                    img = img.resize((300, 350))
                    img = ImageTk.PhotoImage(img)
                    Label(window, image = img).pack()
                window.title("Thông tin sinh viên")
                window.geometry("600x550+10+20")
                window.after(5000, lambda:window.destroy())
                window.mainloop()
        else:
            db.child("informationStudent").update({"sai_nguoi":1})

    return frame, ckd, start

firebaseConfig = {
    'apiKey': "AIzaSyCt60H3fUGiPv973_fMMN51lp2XXRazjF0",
    'authDomain': "arduino-firebase-vippro.firebaseapp.com",
    'databaseURL': "https://arduino-firebase-vippro-default-rtdb.firebaseio.com",
    'projectId': "arduino-firebase-vippro",
    'storageBucket': "arduino-firebase-vippro.appspot.com",
    'messagingSenderId': "437474413862",
    'appId': "1:437474413862:web:92d2b1c67528bf994fc5e7",
    'measurementId': "G-HMRKLR9EVW",
    'serviceAccount': "serviceAccountKey.json"
}

firebase = pyrebase.initialize_app(firebaseConfig)
auth = firebase.auth()
email = "toilaminh@mail.com"
password = "123456"
auth.sign_in_with_email_and_password(email, password)
print("Successfully signed in!")

ckd = None
ser = sr.Serial('COM5', 9600)
cap = cv2.VideoCapture(0)

start = None
while True:
    ret, frame = cap.read()
    end = time.time()
    frame = cv2.flip(frame, 1)
    frame = zoom_center(frame, 1)
    frame, ckd, start = read_qrcode(frame, ckd, start)
    if start != None:
        print(int(end-start))
        if int(end-start) > 20:
            ckd = None
            start = None
    cv2.imshow("QR code reader", frame)

    if cv2.waitKey(1) & 0xFF == ord("m"):
        ser.write("m".encode("ascii"))
    if cv2.waitKey(1) & 0xFF == ord("d"):
        ckd = None      
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()