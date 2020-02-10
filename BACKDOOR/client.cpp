#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

//    Creazione del socket
int CreaSocket(char* destinazione, int porta){
    
    //creo una struttura di tipo sockaddr_in;
    //presa da libreria ma non si sa quale
    struct sockaddr_in temp;

    struct hostent *h; 
    
    //creo il socket;
    int sock; 
  
    //variabile per eventuali errori 
    int errore;

    //Tipo di indirizzo
    temp.sin_family = AF_INET;    
    //Questa funzione si occupa della conversione al formato internet per numeri di tipo "unsigned short int"
    temp.sin_port = htons(porta);     
    h = gethostbyname(destinazione);
    
    //nessuna destinazione trovata
    if (h==0){
    
        cout<<"Creazione socket fallita"<<endl;        
        exit(1);

    } 
    bcopy(h->h_addr,&temp.sin_addr,h->h_length);
         
    //Creazione socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
   
    //Connessione del socket. Esaminare errore
    
    errore = connect(sock, (struct sockaddr*) &temp, sizeof(temp));

    return sock;
}

//    Assegnazione indirizzo
//    Connessione/Attesa di una connessione
//    Scambio dati

//    Chiusura socket 
void ChiudiSocket(int sock){
    close(sock);
    return;
}

int main(){
   
    //variabile per il socket
    int des_sock;
    string app0 = "192.168.1.176";
    char* dest = new char[app0.length()+1];
    strcpy(dest, app0.c_str());    
    cout<<dest<<endl;       
    //Creo e connetto il socket
    des_sock = CreaSocket(dest, 4444);
    
    //Chiudo il socket
    ChiudiSocket(des_sock);
    return 0;
}

