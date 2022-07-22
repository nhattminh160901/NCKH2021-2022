import pyrebase

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
db = firebase.database()
auth = firebase.auth()
email = 'toilaminh@mail.com'
password = '123456'
auth.sign_in_with_email_and_password(email, password)
print("successfully signed in!")

data = {}
def sendata(number):
    data = {"moc":number}
    db.child("cotden").update(data)

sendata(3)