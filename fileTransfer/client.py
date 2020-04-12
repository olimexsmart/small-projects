import socket               # Import socket module

s = socket.socket()         # Create a socket object
host = socket.gethostname()  # Get local machine name
port = 12345                 # Reserve a port for your service.

s.connect((host, port))
# s.send("Hello server!".encode())
f = open('test.png', 'rb')
print('Sending...')
listener = f.read(1024)
while (listener):
    print('Sending...')
    s.send(listener)
    listener = f.read(1024)
f.close()
print("Done Sending")
s.shutdown(socket.SHUT_WR)
print(s.recv(1024).decode())
s.close
