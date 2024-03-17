import tkinter as tk
import socket
import numpy as np
import threading
from PIL import Image, ImageTk

# Załadowanie obrazów planszy i pionków
plansza = Image.open('plansza.png')
pionek_cz = Image.open('img//czarny.png')
pionek_b = Image.open('img//bialy.jpg')

class ChessClient:
    def __init__(self, master):
     # Inicjalizacja interfejsu użytkownika
        self.master = master
        self.master.title("Szachy - Klient")

        self.board_canvas = tk.Canvas(self.master, width=600, height=600)
        self.board_canvas.pack()

        self.message_label = tk.Label(self.master, text="")
        self.message_label.pack()

        self.move_entry = tk.Entry(self.master)
        self.move_entry.pack()

        self.send_button = tk.Button(self.master, text="Wyslij ruch", command=self.send_move)
        self.send_button.pack()
        
        # Inicjalizacja gniazda klienta i połączenie z serwerem
        self.clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect_to_server()
        
        # Inicjalizacja danych planszy i obrazu
        self.board_data = None
        self.photo = None

        # Wątek do odbierania danych od serwera
        self.receive_thread = threading.Thread(target=self.receive_data)
        self.receive_thread.start()
        
        #Spawdzanie aktualzacji co 1mili
        self.master.after(1, self.check_for_updates)

    def check_for_updates(self):
        # Sprawdź, czy są nowe dane do wyświetlenia
        if self.board_data is not None:
            self.show_board()

        # Ponownie uruchom funkcję po 100 milisekundach
        self.master.after(100, self.check_for_updates)

    def connect_to_server(self):
    # Połączenie z serwerem na podstawie podanych przez użytkownika danych
        port = input("Podaj numer portu serwera: ")
        port = int(port)
        address = input("Podaj adres serwera: ")
        address = str(address)
        serverAddr = (address, port)
        self.clientSocket.connect(serverAddr)

    def send_move(self):
    # Wysłanie ruchu do serwera
        move = self.move_entry.get()
        self.clientSocket.send(move.encode())

    def show_board(self):
    # Wyświetlenie planszy na interfejsie użytkownika
        rozmiar_pola = 1
        new_image = plansza.copy()

        for i in range(0, 74*8, 74):
            for j in range(0, 74 * 8, 74):
                if self.board_data[i//74][j//74] == -1:
                    new_image.paste(pionek_cz, ((j+10) * rozmiar_pola, (i+10) * rozmiar_pola))
                elif self.board_data[i//74][j//74] == 1:
                    new_image.paste(pionek_b, ((j+10) * rozmiar_pola, (i+10) * rozmiar_pola))
        # Utworzenie nowego obiektu PhotoImage i aktualizacja widoku planszy
        self.photo = ImageTk.PhotoImage(new_image)
        self.board_canvas.create_image(0, 0, anchor=tk.NW, image=self.photo)
        self.master.update_idletasks()

    def receive_data(self):
     # Odbieranie danych od serwera
        while True:
            buffer = self.clientSocket.recv(1024)
            message = buffer.decode('latin-1').strip('\x00')

            if message == "wprowadz wsp":
                print("Wprowadź ruch np.: c7-c6")
            elif message == "sending board":
            # Odbieranie danych planszy i przekształcanie ich do odpowiedniej formy
                board_data = self.clientSocket.recv(256)
                board_data = np.frombuffer(board_data, dtype=np.int32)
                board_data = board_data.reshape((8, 8))
                # Aktualizacja danych planszy
                self.board_data = board_data
            elif message == "info from server":
            # Odbieranie informacji od serwera
                buffer = self.clientSocket.recv(1024).decode()
                print("Serwer wysłał: ", buffer)
            elif message == "exit":
                break
            elif message == "disconnect":
                print("Przeciwnik uciekł wygrałeś!")
                break

        # Zamknięcie gniazda po zakończeniu pętli
        self.clientSocket.close()

def main():
# Inicjalizacja aplikacji klienta
    root = tk.Tk()
    client_app = ChessClient(root)
    root.mainloop()

if __name__ == "__main__":
    main()