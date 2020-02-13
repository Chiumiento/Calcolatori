/********Dichiarazione assembler di un processo ********/

        .extern c_processo
a_processo: 
        /* salva lo stato del processore nel descrittore del processo identificato dalla variabile esecuzione, salva anche (nel campo constesto[I_RSP] del descrittore di processo) il contenuto del registro RSP come lasciato dal processore dopo il cambio di pila e il successivo salvataggio in questa delle cinque parole lunghe. */
        call salva_stato       
        
        call c_processo         
        
        /* carica nel processore lo stato contenuto nel descrittore del processo identificato da esecuzione, ripristinerà il registro RSP affinchè la iretq finale possa estrarre dalla pila sistema queste informazioni, facendo dunque proseguire il processo dal punto in cui era stato interrotto. */
        call carica_stato       

        iretq 

/******** ********/


/******** Coda di processi ********/

struct proc_elem {
        natl id;        // identificatore del processo
        natl precedenza;        // priorità del processo
        proc_elem *puntatore;       // puntatore al prossimo processo in coda
};
proc_elem *esecuzione;      // processo in esecuzione
proc_elem *pronti;      // coda dei processi pronti per essere eseguiti

/******** ********/



/******** Descrittore di processo ********/
 
struct des_proc {
        // parte richiesta dall'hardware
        struct __attribute__ ((packed)) {
                natl riservato1;        
                vaddr punt_nucleo;      // punta alla pila sistema di P 

                // due quad  a disposizione (puntatori alle pile ring 1 e 2)
                natq disp1[2];      
                natq riservato2;        

                //entry della IST, non usata
                natq disp2[7];
                natq riservato3; 
                natw riservato4;
                natw iomap_base;        // si veda crea_processo()
        };
        //finiti i campi obbligatori
        faddr cr3;      // numero del frame che contiene la tabella di livello 4 di quel processo
        natq contesto[N_REG];       // quì viene memorizzato il contenuto dei registri del processore [ARRAY]
        natl cpl;       // Current Privilege Level (privilegio attuale del processore)
};

// numero di processi utente attivi
volatile natl processi;
// distrugge il processo puntato da esecuzione
extern "C" void c_abort_p();
// dato un id, restiusce il puntatore al corrispondente des_proc
// (definita in sistema.s)
extern "C" des_proc *des_p(natl id);
// id del procsso dummy (creato durante l'inizializzazione)
natl dummy_proc;        // conta il numero di processi esistenti nel sistema, quando sono tutti terminati esegue lo shutdown del sistema

/******** ********/



/******** Macro cavallo_di_troia ********/

/* queste devono essere usate nella parte assembler della primitiva, prima di chiamare la parte C++. Se la parte assembler chiama salva_stato, queste macro devono essere chiamate dopo, in quanto sporcano alcuni registri */

/* richiede un parametro, che deve essere un operando assembler; controlla che l'operando contenga un indirizzo a cui l'utente può accedere */
cavallo_di_troia %rdi // reg, %rdi se è il primo parametro della funzione

/* richiede due parametri, ciascuno dei quali deve essere un operando assembler; il primo parametro ha lo stesso significato di prima, mentre il secondo deve specificare la lunghezza della zona di memoria puntata */
cavallo_di_troia %rdi %rsi // base dimensione, %rdi ed %rsi se sono i primi due parametri della funzione

/******** ********/



/******** Scrivere nuove primitive ********/

// costanti.h
/* per prima cosa occcorre assegnare un tipo di interruzione alla primitiva modificando il file costanti.h, supponiamo di voler aggiungere una primitiva getid() che restituisce l'identificatore del processo che la invoca */

#define TIPO_GETID  0x59 // 0x59 semplicemente perchè non è ancora stato usato

// sistema.s
/* per caricare il corrispondente gate della IDT possiamo aggiungere una riga alla funzione init_idt che si trova nel file sistema.s. Tale funzione è chiamata all'avvio del sistema e si occupa di inizializzare la tabella IDT. Possiamo usare la macro carica_gate che richiede tre parametri: tipo della primitiva, indirizzo a cui saltare, livello di privilegio per utilizzarla tramite una int (DPL);
P = 1; I/T = T; L = S; */

carica_gate TIPO_GET_ID a_getid LIV_UTENTE

/* non utilizzo la salva_stato e la carica_stato poichè questo processo verrà sempre eseguito interamente */

    .extern c_getid
a_getid:
    call c_getid
    iretq

//sistema.cpp
/* quì scriveremo la primitiva vera e propria, in caso di funzioni che richiedono l'utilizzo della salva_stato e della carica_stato avrei dovuto dichiarare la primitiva di tipo void ed inserire il ritorno nel contesto del processo. Questo perchè il registro %RAX è inutilizzabile in quanto verrà poi riscritto con la carica_stato perdendo il ritorno voluto */
extern "C" natl c_getid(){
    return esecuzione->id;
}

//sys.h

extern "C" natl getid();

//utente.s

.global getid
getid:
    int $TIPO_GETID
    ret
 
/******** ********/



/******** Funzioni di supporto ********/
 
// restituisce un puntatore al descrittore del processo di identificatore id (0 se tale processo non esiste)
des_proc *des_p(natl id)

// sceglie il prossimo persorso da mettere in esecuzione (cambiando quindi il valore della variabile esecuzione)
void schedulatore()

// inserisce p_elem nella lista p_lista, mantenendo l'ordinamento basato sul campo precedenza. Se la lista contiene altri elementi che hanno la stessa precedenza del nuovo, il nuovo viene inserito come ultimo tra questi
void inserimento_lista(proc_elem *&p_lista, proc_elem *p_elem)

//estrae l'elemento in testa alla lista p_lista e ne restituisce un puntatore in p_elem
void rimozione_lista(proc_elem *&p_lista, proc_elem *&p_elem)

//inscerisce il proc_elem puntato da esecuzione in testa alla coda pronti
inspronti();

//distrugge il processo corrente e salta ad un altro. Attenzione: la funzione non ritorna al chiamante
abort_p()

/******** ********/



/************* SEMAFORI *********/

/* Essenzialmente nascono dal fatto che più processi operano concorrentemente in un sistema. Nasce dunque il problema dell'interferenza,questo vuol dire che mentre un processo
sta eseguendo delle operazioni su una struttura dati comune, un altro processo potrebbe inserirsi e cominciare anche lui a modificare la stessa struttura dati.
Al problema dell'interferenza e quindi all'esigenza di un coordinamento dei vari processi su strutture dati in comune, si risponde con la MUTUA ESCLUSIONE: mentre è in corso un'operazione
su una struttura dati non ne può partire nessun'altra. Un altro problema è quello della SINCRONIZZAZIONE tra i processi: un utente può aver bisogno che un'azione B avvenga per forza e sempre
dopo un'azione A.
ATTENZIONE: la mutua esclusione è ben diversa dalla sincronizzazione, infatti nella seconda vogliamo garantire un ordinamento tra le azioni che si compiono, nella mutua esclusione
vogliamo garantire che l'azione su una struttura dati sia l'unica in quell'istante.

I due problemi di mutua esclusione e sincronizzazione si risolvono supponendo di avere delle scatole che possono contenere dei gettoni tutti uguali.
Solo due operazioni possono essere eseguite su queste scatole:
-> INSERIMENTO DI UN GETTONE, non è necessario che sia stato precedentemente preso da una scatola
-> PRENDERE UN GETTONE, se non ve ne sono bisogna aspettare che qualcuno lo inserisca.

Nel nostro sistema, quelle che finora abbiamo chiamato SCATOLE, le denominiamo più precisamente SEMAFORI, per motivi storici.
Forniamo dunque delle primitive all'utente che può utilizzare per garantire il coordinamento tra i processi.

//inizializza un semaforo (una scatola) con v risorse (v gettoni) e ne resituisce l'identificatore (o 0XFFFFFFFF se non è stato possibile crearlo)*/
natl sem_ini(natl v);

//prende un gettone (una risorsa) dal semaforo sem, blocca il processo chiamante se non ve ne sono di disponibili
void sem_wait(natl sem);

//inserisce un gettone (una risorsa) nel semaforo sem, risveglia eventuali processi bloccati in attesa di un gettone (una risorsa)
void sem_signal(natl sem);

//Ora vediamo come risolvere i problemi di mutua e sincronizzazione facendo uso di tali primitive descritte

//MUTUA ESCLUSIONE
//inizializao un semaforo con una risorsa (un gettone)
natl mutex = sem_ini(1);
//Ora tutte le azioni di un processo vengono protette in questo modo
Processo:
    sem_wait(mutex); //attendo che l'oggetto su cui voglio eseguire l'operazione sia libero
    -> AZIONE //eseguo l'operazione
    sem_signal(mutex); //segnalo che ora l'oggetto è libero di nuovo

//SINCRONIZZAZIONE
natl sync = sem_ini(0);

//dati PA e PB due processi che dobbiamo sincronizzare
PA:
    ->Azione   
    sem_signal(sync) //avviso che la condizione per fare scattare PB è vera

PB:
    sem_wait(sync) //attendo che la condizione per farmi scattare sia vera 
    ->AZIONE

//Prevediamo una struttura descrittiva dei semafori nel nucleo
struct des_sem{
    int counter; //il numero dei gettoni che il semaforo contiene
    proc_elem* pointer; //processi in attesa su quel semaforo
};

// ( [P_SEM_ALLOC]
// I semafori non vengono mai deallocati, quindi e' possibile allocarli
// sequenzialmente. Per far questo, e' sufficiente ricordare quanti ne
// abbiamo allocati
natl sem_allocati = 0;
natl alloca_sem()
{
        natl i;

        if (sem_allocati >= MAX_SEM)
                return 0xFFFFFFFF;

        i = sem_allocati;
        sem_allocati++;
        return i;
}

// dal momento che i semafori non vengono mai deallocati,
// un semaforo e' valido se e solo se il suo indice e' inferiore
// al numero dei semafori allocati
bool sem_valido(natl sem)
{
        return sem < sem_allocati;
}

// parte "C++" della primitiva sem_ini
extern "C" natl c_sem_ini(int val)
{
        natl i = alloca_sem();

        if (i != 0xFFFFFFFF)
                array_dess[i].counter = val;

        return i;
}
// )

extern "C" void c_sem_wait(natl sem)
{
        des_sem *s;

// (* una primitiva non deve mai fidarsi dei parametri
        if (!sem_valido(sem)) {
                flog(LOG_WARN, "semaforo errato: %d", sem);
                c_abort_p();
                return;
        }
// *)

        s = &array_dess[sem];
        (s->counter)--;

        if ((s->counter) < 0) {
                inserimento_lista(s->pointer, esecuzione);
                schedulatore();
        }
}

extern "C" void c_sem_signal(natl sem)
{
        des_sem *s;
        proc_elem *lavoro;

// (* una primitiva non deve mai fidarsi dei parametri
        if (!sem_valido(sem)) {
                flog(LOG_WARN, "semaforo errato: %d", sem);
                c_abort_p();
                return;
        }
// *)

        s = &array_dess[sem];
        (s->counter)++;

        if ((s->counter) <= 0) {
                rimozione_lista(s->pointer, lavoro);
                inspronti();    // preemption
                inserimento_lista(pronti, lavoro);
                schedulatore(); // preemption
        }
}

/**************************** FINE SEMAFORI **************************************************/




/******** Memoria Virtuale ********/

// questa istruzione cambia potenzialmente tutta la tabella di livello 4 usata fino al momento prima, quindi tutte le informazioni contenuto nel TLB(cache dell'MMU) sono da considerarsi non più valide e l'intero TLB viene svuotato.
movq %rax, %cr3

// questa istruzione dice al TLB di invalidare la traduzione relativa all'indirizzo dell'operando passato come argomento
invlpg operando_in_memoria

// la routine di page fault deve conoscere l'indirizzo V che ha causato il fault, a tale scopo introduciamo un registro speciale del processore chiamato %cr2. Aggiungiamo poi questa nuova istruzione che la routine di page fault può eseguire per copiare l'indirizzo V in %rax in modo che poi lo possa elaborare liberamente con le istruzioni già esistenti.
mov %cr2, %rax

// istruzioni per invalidare il TLB
movq %cr4, %rax;
movq %rax, %cr3;

// avremo un descrittore di frame per ogni frame della parte M2.  Lo scopo del descrittore e' di contenere alcune informazioni relative al contenuto del frame corrispondente. Tali informazioni servono principalmente a facilitare o rendere possibile il rimpiazzamento del contenuto stesso.
struct des_frame {
        int     livello;        // 0=pagina, -1=libera, >0=livello tabella
        bool    residente;      // pagina residente o meno
        // identificatore del processo a cui appartiene l'entita'
        // contenuta nel frame.
        natl    processo;
        natl    contatore;      // contatore per le statistiche
        // blocco da cui l'entita' contenuta nel frame era stata caricata
        natq    ind_massa; 
        // per risparmiare un po' di spazio uniamo due campi che
        // non servono mai insieme:
        // - ind_virtuale serve solo se il frame e' occupato
        // - prossimo_libero serve solo se il frame e' libero
        union {
                // indirizzo virtuale che permette di risalire al
                // descrittore che punta all'entita' contenuta nel
                // frame. Per le pagine si tratta di un qualunque
                // indirizzo virtuale interno alla pagina. Per le
                // tabelle serve un qualunque indirizzo virtuale la
                // cui traduzione passa dalla tabella.
                vaddr   ind_virtuale;
                des_frame*      prossimo_libero;
        };
};

des_frame* vdf;         // vettore di descrittori di frame
                        // (allocato in M1, si veda init_dpf())
faddr primo_frame_utile;        // indirizzo fisico del primo frame di M2
natq N_DF;                      // numero di frame in M2
des_frame* frame_liberi;        // indice del descrittore del primo frame libero

/* routine di page fault, ricordiamo che al termine di un fault viene rieseguita l'istruzione che lo ha causato.
*/
bool c_routine_pf()
{
        vaddr ind_virt = readCR2();     // legge dal registro %cr2 l'indirizzo che la MMU non è riuscita a tradurre, quindi carica la pagine che lo contiene e tutte le tabelle necessarie per la traduzione
        
        natl proc = esecuzione->id;     
    
        stat();
 
        for (int i = 3; i >= 0; i--) {      // ciclo che va dal livello 3 al livello 0
                tab_entry d = get_des(proc, i + 1, ind_virt);       // per ogni livello preleva il corrispondente descrittore
                bool bitP = extr_P(d);      //estrae il bit P
                if (!bitP) {        // se l'entità non è presente
                        des_frame *df = swap(proc, i, ind_virt);        // la carica
                        if (!df)        // la funzione fallisce ritornando un puntatore nullo 
                                return false;
                }
        }
        return true;
}



/******** Fine Memoria Virtuale ********/8









































