import socket               # Import socket module

s = socket.socket()         # Create a socket object
host = "0.0.0.0"  # Get local machine name
port = 12345                 # Reserve a port for your service.
s.bind((host, port))        # Bind to the port
f = open('torecv.mp4', 'wb')
s.listen(5)                 # Now wait for client connection.
while True:
    c, addr = s.accept()     # Establish connection with client.
    print('Got connection from', addr)
    print("Receiving...")
    listener = c.recv(1024)
    while (listener):
        print("Receiving...")
        f.write(listener)
        listener = c.recv(1024)
    f.close()
    print("Done Receiving")
    c.send("Thank you for connecting".encode())
    c.close()
