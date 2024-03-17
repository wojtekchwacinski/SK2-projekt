import socket
import sys
import numpy as np
from PIL import Image, ImageDraw
board = [[0 for _ in range(8)] for _ in range(8)]
plansza = Image.open('plansza.png')
pionek_b = Image.open('img//bialy.jpg')
pionek_cz = Image.open('img//czarny.png')
def show_board(board):
    print("    A   B   C   D   E   F   G   H")
    print("  ---------------------------------")
    for i in range(7, -1, -1):
        print("  ---------------------------------")
        print(f"{i + 1} |", end="")
        for j in range(8):
            if board[i][j] == -1:
                print(" ♟︎ |", end="")
            elif board[i][j] == 0:
                print("    |", end="")
            elif board[i][j] == 1:
                print(" ♙ |", end="")
        print()
    print("  ---------------------------------")
    print("    A   B   C   D   E   F   G   H")

def show_board_to_client(board,plansza):
    # Rozmiar pojedynczego pola na szachownicy
    plansza = Image.open('plansza.png')
    rozmiar_pola = 1
    pozycje_pionkow = board
    # Rysujemy pionki na szachownicy na nowym obrazie
    nowy_obraz = board.copy()


    for i in range(0, 74*8, 74):

        for j in range(0, 74 * 8, 74):
            if board[i//74][j//74] == -1:
                plansza.paste(pionek_cz, ((j+10) * rozmiar_pola, (i+10) * rozmiar_pola))
            elif board[i//74][j//74] == 1:
                plansza.paste(pionek_b, ((j+10) * rozmiar_pola, (i+10) * rozmiar_pola))
    plansza.show()

def main():

    clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    port = input("proszę o podanie portu servera")
    port = int(port)
    adres = input("proszę o podanie adresu servera")
    adres = str(adres)
    serverAddr = (adres, port)
    clientSocket.connect(serverAddr)
    addr_size = clientSocket.getsockname()

    while True:
        #plansza = Image.open('chessboard.jpg')

        buffer = []
        while not buffer:

            buffer = clientSocket.recv(1024)

        komunikat = []
        for item in buffer.decode('latin-1').split("\n"):
            komunikat = item
            break
        buffer = str(buffer)
        #print(buffer[2:])
        dobry = komunikat.strip('\x00')
        print(dobry)
        #print(type(dobry))
        #print(komunikat)
        #for item in dobry:
            #print("1", item)
        if dobry == "wprowadz wsp":
            print("Wporwadz następny ruch np.:c7-c6")
            message = input()
            clientSocket.send(message.encode())
        elif dobry == "sending board":
            print("Plansza: ")
            board_data = clientSocket.recv(256)
            recived = np.frombuffer(board_data, dtype=np.int32)
            recived = recived.reshape((8,8))
            show_board(recived)
            show_board_to_client(recived, plansza)
        elif dobry == "info from server":
            print("Serwer wysłał: ", end="")
            buffer = clientSocket.recv(1024).decode()
            print(buffer)
        elif dobry == "exit":
            break
        elif dobry == "disconnect":
            print("Przeiwnik uciekł wygrałeś!")
            break

            


    clientSocket.close()


if __name__ == "__main__":
    main()

