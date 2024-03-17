#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include<pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h> 
#define MAX_GAMES 3

char buf[1024];
char from_client[1024];
// Definicje plansz dla trzech gier
//1 to biały pion a -1 to czarny pion 0 to puste pole
int plansze[3][8][8]= {
    {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0}
    },
        {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0}
        }
};
// Informacje o grach
int games[3] = {0, 0, 0};
int game_status[3] = {0, 0, 0};
int ruch[3] = {0, 0, 0};


void pionek_bialy(int plansza[8][8], int x1, int y1, int x2,  int y2){
    x1--;
    y1--;
    x2--;
    y2--;
    plansza[x1][y1] = 0;
    plansza[x2][y2] = 1;
}
void pionek_czarny(int plansza[8][8], int x1, int y1, int x2,  int y2){
    x1--;
    y1--;
    x2--;
    y2--;
    plansza[x1][y1] = 0;
    plansza[x2][y2] = -1;
}




int char_to_index(char c) {
    return c - 'a' + 1;
}
bool hasWhiteWon(int board[8][8]) {
    // Sprawdź, czy nie ma już czarnych pionków na planszy
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == -1) {
                return false;
            }
        }
    }
    return true;
}
bool hasWhiteWonEndLine(int board[8][8]){
        for(int j = 0; j < 8; ++j){
                if(board[0][j] == 1) {
                        return true;
                }
        }
        return false;

}

bool canWhitePawnMoveTo(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    fromRow--;
    fromCol--;
    toRow--;
    toCol--;
	
	if (board[fromRow][fromCol] != 1) {
        return false;
    }
	
    if (toRow == fromRow - 1 && toCol == fromCol && board[toRow][toCol] == 0) {
        return true; // Biały pion porusza się do przodu o jedno pole
    }

    if (toRow == fromRow - 1 && (toCol == fromCol - 1 || toCol == fromCol + 1) && board[toRow][toCol] < 0) {
        return true; // Biały pion wykonuje bicie
    }

    return false;
}

bool canBlackPawnMoveTo(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    fromRow--;
    fromCol--;
    toRow--;
    toCol--;


	if (board[fromRow][fromCol] != -1) {
        return false;
    }
    // Sprawdź, czy czarny pion znajdujący się na (fromRow, fromCol) może przesunąć się na (toRow, toCol)
    if (toRow == fromRow + 1 && toCol == fromCol && board[toRow][toCol] == 0) {
        return true; // Czarny pion porusza się do przodu o jedno pole
    }

    if (toRow == fromRow + 1 && (toCol == fromCol - 1 || toCol == fromCol + 1) && board[toRow][toCol] > 0) {
        return true; // Czarny pion wykonuje bicie
    }

    // Dodaj warunek dla dotarcia na drugi koniec szachownicy, jeśli toRow jest równe 7

    return false;
}

bool hasBlackWon(int board[8][8]) {
    // Sprawdź, czy nie ma już białych pionków na planszy
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == 1) {
                return false;
            }
        }
    }
    return true;
}
bool hasBlackWonEndLine(int board[8][8]){
        for(int j = 0; j < 8; ++j){
                if(board[7][j] == -1){
                        return true;
                }
        }
        return false;
}

void * socketThread(void *arg){\
    // Pobranie numeru gniazda dla nowego gracza
    int newSocket = *((int *)arg);
    printf("Nowy gracz na serwerze %d\n", newSocket);
    int n;
    int game_found = 0;
    char color[128];
    int game_id = -1;
    int x_1, x_2, y_1, y_2;
    char x_1_char, x_2_char;
	
    // Sprawdzenie dostępności miejsca w grze
    for (int i = 0; i < MAX_GAMES;i++){
        if(games[i]<2){
        game_id = i;
        game_found = 1;
		// Ustalenie koloru gracza w zależności od dostępnego miejsca w grze
        if(games[i]==1){
             strcpy(color, "czarne");
        }
        else if(games[i]==0){
            strcpy(color, "biale");
        }
        games[i]++;
        break;
        }
    }
	// Obsługa przypadku, gdy nie ma dostępnego miejsca w grze
    if(game_found==0){
        send(newSocket, "info from server", sizeof("info from server"), 0);
        strcpy(buf, "Osiagnieto maksymalna liczba graczy");
        send(newSocket, buf, sizeof(buf), 0);
        memset(&buf, 0, sizeof(buf));
        printf("Exit %d\n",newSocket);
        send(newSocket, "exit", sizeof("exit"), 0);
        close(newSocket);
        pthread_exit(NULL);
    }
    else{
		// Informowanie gracza o dołączeniu do gry
        write(newSocket, "info from server", sizeof("info from server"));
        memset(&buf, 0, sizeof(buf));
        strcpy(buf, "Jesteś w grze");
        write(newSocket, buf, sizeof(buf));
        memset(&buf, 0, sizeof(buf));
	}
	write(newSocket, "info from server", sizeof("info from server"));
	memset(&buf, 0, sizeof(buf));

	strcpy(buf, color);
	write(newSocket, buf, sizeof(buf));
	memset(&buf, 0, sizeof(buf));
	char game_id_string[128];
	sprintf(game_id_string, "%d", game_id);

	write(newSocket, "info from server", sizeof("info from server"));
	memset(&buf, 0, sizeof(buf));
	strcpy(buf, game_id_string);
	write(newSocket, buf, sizeof(buf));
	memset(&buf, 0, sizeof(buf));
    



	// Główna pętla obsługująca ruchy graczy
    for (;;){
		// Sprawdzenie, czy gra została przerwana
        if(game_status[game_id]==3){
            send(newSocket, "disconnect", sizeof("disconnect"), 0);
            game_status[game_id] = 0;
            games[game_id] = 0;
            for (int i = 0; i<8;i++){
                for (int j = 0; j < 8;j++){
                    plansze[game_id][i][j] = plansze[0][i][j];
                }
            }
				ruch[game_id] = 0;

        }
		// Obsługa ruchu gracza białego
        else if (ruch[game_id] == 0 && strcmp(color,"biale")==0) {
                while (1){
					// Informowanie gracza o konieczności wprowadzenia ruchu
                    send(newSocket, "wprowadz wsp", sizeof("wprowadz wsp"), 0);
                    n = read(newSocket, from_client, sizeof(from_client));
                    if(n<=0){
						// Obsługa rozłączenia gracza
                        game_status[game_id] = 3;
                        printf("Klient postnowił się rozłączyć: %d\n", newSocket);
                        close(newSocket);
                        pthread_exit(NULL);
                        break;
                    }
					// Odczyt współrzędnych ruchu od gracza
                    sscanf(from_client, "%c%d-%c%d", &x_1_char, &x_1, &x_2_char, &x_2);
                    y_1 = char_to_index(x_1_char);
                    y_2 = char_to_index(x_2_char);
					// Sprawdzenie poprawności ruchu białego piona
                    if (canWhitePawnMoveTo(plansze[game_id], x_1, y_1, x_2, y_2)==true){
						// Wykonanie ruchu i wysłanie planszy obu graczom
                        pionek_bialy(plansze[game_id],x_1,y_1,x_2,y_2);
                        send(newSocket, "sending board", sizeof("sending board"), 0);
                        send(newSocket, plansze[game_id], sizeof(plansze[game_id]), 0);
                        send(newSocket+1, "sending board", sizeof("sending board"), 0);
                        send(newSocket+1, plansze[game_id], sizeof(plansze[game_id]), 0);						

                        ruch[game_id] = 1;
                        break;
                    }
                }
            }
			// Obsługa ruchu gracza czarnego
        else if (ruch[game_id] == 1 && strcmp(color,"czarne")==0) {
                while (1){
					// Informowanie gracza o konieczności wprowadzenia ruchu
                    send(newSocket, "wprowadz wsp", sizeof("wprowadz wsp"), 0);
                    n = recv(newSocket, from_client, sizeof(from_client),0);
                    if(n<=0){
						// Obsługa rozłączenia gracza
                        game_status[game_id] = 3;
                        printf("Klient postanowił się rozłaczyć: %d\n", newSocket);
                        close(newSocket);
                        pthread_exit(NULL);
                        break;
                    }
					// Odczyt współrzędnych ruchu od gracza
                    sscanf(from_client, "%c%d-%c%d", &x_1_char, &x_1, &x_2_char, &x_2);
                    y_1 = char_to_index(x_1_char);
                    y_2 = char_to_index(x_2_char);
                    if (canBlackPawnMoveTo(plansze[game_id],x_1,y_1,x_2,y_2)==true){
						// Sprawdzenie poprawności ruchu czarnego piona
                        pionek_czarny(plansze[game_id],x_1,y_1,x_2,y_2);
                        send(newSocket, "sending board", sizeof("sending board"), 0);
                        send(newSocket, plansze[game_id], sizeof(plansze[game_id]), 0);
                        send(newSocket-1, "sending board", sizeof("sending board"), 0);
                        send(newSocket-1, plansze[game_id], sizeof(plansze[game_id]), 0);
                        ruch[game_id] = 0;
                        break;
                    }
                }
            }
		// Obsługa przypadków zwycięstwa białego gracza
        else if(hasWhiteWon(plansze[game_id]) == true || hasWhiteWonEndLine(plansze[game_id]) == true){
            send(newSocket, "info from server", sizeof("info from server"), 0);
            send(newSocket, "Biały wygrał", sizeof("Biały wygrał"), 0);
			// Zresetowanie planszy i zakończenie gry
            for (int i = 0; i<8;i++){
                for (int j = 0; j < 8;j++){
                    plansze[game_id][i][j] = plansze[0][i][j];
                }
            }
            ruch[game_id] = 0;
            games[game_id] = 0;
            game_status[game_id] = 0;
            break;
        }
		// Obsługa przypadków zwycięstwa czarnego gracza
        else if(hasBlackWon(plansze[game_id]) == true || hasBlackWonEndLine(plansze[game_id]) == true){
            send(newSocket, "info from server", sizeof("info from server"), 0);
            send(newSocket, "Czarny wygrał", sizeof("Czarny wygrał"), 0);
			
			// Zresetowanie planszy i zakończenie gry
            for (int i = 0; i<8;i++){
                for (int j = 0; j < 8;j++){
                    plansze[game_id][i][j] = plansze[0][i][j];
                }
            }
			
            ruch[game_id] = 0;
            games[game_id] = 0;
            game_status[game_id] = 0;
            break;
        }

    }
		// Zakończenie wątku, wysłanie informacji o zakończeniu gry, zamknięcie gniazda i zakończenie wątku
        printf("Exit socketThread %d\n",newSocket);
        send(newSocket, "exit", 4, 0);
        close(newSocket);
        pthread_exit(NULL);

}


int main(){
  // Deklaracje zmiennych dla socketów i struktur adresowych
  int sfd, newSocket;
  struct sockaddr_in saddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  
  // Utworzenie gniazda
  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  // Inicjalizacja struktury adresowej
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(1234);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //wyczyszczeie portu
  memset(saddr.sin_zero, '\0', sizeof saddr.sin_zero);
  // Przypisanie adresu i portu do gniazda
  bind(sfd, (struct sockaddr *) &saddr, sizeof(saddr));
  // Nasłuchiwanie na połączenia
  if(listen(sfd,10)==0)
    printf("Listening\n");
  else{
    printf("Error\n");
  }
    pthread_t thread_id;

    while(1)
    {	 
		// Akceptowanie nowego połączenia
        addr_size = sizeof serverStorage;
        newSocket = accept(sfd, (struct sockaddr *) &serverStorage, &addr_size);
		
		// Utworzenie nowego wątku do obsługi połączenia z klientem
        if( pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0 )
           printf("Failed to create thread\n");
	   
		// Odłączenie wątku od procesu głównego
        pthread_detach(thread_id);
  
    }
  
  return 0;
}