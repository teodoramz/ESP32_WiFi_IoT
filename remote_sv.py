import socket
import subprocess
import os

def run_hashcat(pmkid, wordlist='rockyou.txt'):
    with open('pmkid.txt', 'w') as f:
        f.write(pmkid)

    command = f"hashcat -m 16800 pmkid.txt {wordlist} --force -o cracked.txt"
    subprocess.run(command, shell=True)

    if os.path.exists('cracked.txt'):
        with open('cracked.txt', 'r') as f:
            result = f.read()
        return result.strip()
    return "Password not found..."

def main():
    host = '0.0.0.0'
    port = 5000

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print(f"Server is listening {host}:{port}")

    try:
        while True:
            client_socket, addr = server_socket.accept()
            print(f"Connection from {addr}")
            pmkid = client_socket.recv(1024).decode().strip()
            print(f"PMKID received: {pmkid}")

            #password = run_hashcat(pmkid)
            password = "NotWorkingYet";
            print(f"Password found: {password}")

            client_socket.sendall(password.encode())
            client_socket.close()
    except KeyboardInterrupt:
        print("Server is closed...")
        server_socket.close()

if __name__ == "__main__":
    main()
