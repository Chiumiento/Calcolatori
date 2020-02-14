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
    
        stat();     // aggiorna le statistiche di utilizzo delle pagine e tabelle presenti
 
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

// routine di rimpiazzamento
// carica l'entita' del livello specificato, relativa all'indirizzo virtuale ind_virt nello spazio di indirizzamento di proc
des_frame* swap(natl proc, int livello, vaddr ind_virt)
{
        tab_entry e = get_des(proc, livello + 1, ind_virt);     // prova ad allocare un frame vuoto destinato a contenere l'entità da caricare usando la funzione alloca_frame(); 
        natq m = extr_IND_MASSA(e);     //la funzione  restituisce un puntatore al descrittore del frame allocato
        if (!m) {       // se l'allocazione fallisce l'entita non può essere caricata e anche lo swap fallisce
                flog(LOG_WARN,
                     "indirizzo %p fuori dallo spazio virtuale allocato",
                     ind_virt);
                return 0;
        }
        des_frame* df = alloca_frame(proc, livello, ind_virt);      // estraiamo l'indirizzo del descrittore dell'entità in memoria di massa
        if (!df) {      // se questo è 0 viol dire che l'indirizzo è fuori dallo spazio virtuale allocato al processo
                flog(LOG_WARN, "memoria esaurita");
                return 0;       // quindi lo swap fallisce
        }
        // riempiamo quindi i campi del descrittore con le informazioni relative all'entità da caricare
        df->livello = livello;      // il suo livello
        df->residente = 0;          // marchiamo l'entità come non residente
        df->processo = proc;        // il processo a cui appartiene
        df->ind_virtuale = ind_virt;        // l'indirizzo virtuale per la cui traduzione la stiamo caricando
        df->ind_massa = m;      // l'indirizzo in memoria di massa appena ottenuto
        df->contatore = 0;      // contatore per le statistiche di utilizzo che verrà poi aggiornato alla prima chiamata della funzione stat();
        carica(df);             // carichiamo l'entità dalla memoria di massa nel frame
        collega(df);            // e la colleghiamo
        return df;
}


// alloca un frame destinato a contenere l'entita' del livello specificato, relativa all'indirizzo virtuale ind_virt nello spazio di indirizzamento di proc
des_frame* alloca_frame(natl proc, int livello, vaddr ind_virt)
{
        des_frame *df = alloca_frame_libero();      // prova ad allocare un frame che sia già libero
        if (df == 0) {      // se non ve ne sono è necessario liberarne uno scegliendo come vittima una delle entità che si trova in questo momento in memoria fisica
                df = scegli_vittima(proc, livello, ind_virt);       // in casi estremi potrebbe non essere possibile liberare alcun frame, questa quindi fallisce
                if (df == 0)        // e di conseguenza anche la alloca_frame è destinata a fallire
                        return 0;   
                bool occorre_salvare = scollega(df);        // per liberare il frame occupato dalla vittima è necesario prima scollegare la vittima, vale a dire porre a 0 il bit P nel descrittore di pagina o tabella la punta
                if (occorre_salvare)        // ed eventualmente
                        scarica(df);        // la scarica
        }
        return df;      // alla fine di questo processo possiamo riutilizzare il frame
}

// routine di selezione vittima
/* è da chiamare solo quando tutti i frame sono occupati, sceglie come vittima quello che ha il contatore minore.
L'algoritmo è dunque una semplice ricerca lineare del minimo, con tre problemi da evitare:
1. non si possono scegliere le pagine marcate come residenti
2. non si possono scegliere le pagine contenenti tabelle che si trovano nello stesso percorso di traduzione dell'entità che stiamo caricando
3. non si possono scegliere tabelle che non siano "vuote"
restituisce true se il frame descritto da df contiene una tabella appartenente allo stesso percorso (per farlo controlla i bit opportuni del campo ind_virtuale del descrittore).
*/ 

des_frame* scegli_vittima(natl proc, int liv, vaddr ind_virt)
{
        des_frame *df, *df_vittima;
        df = &vdf[0];
        // nelle prossime 4 righe la funzione cerca un primo valore da usare come minimo di partenza 
        while ( df < &vdf[N_DF] &&
                (df->residente ||
                 vietato(df, proc, liv, ind_virt)))     
                df++;
        if (df == &vdf[N_DF]) return 0;
        df_vittima = df;    
        // scorre tutti i rimanenti descrittori di frame alla ricerca di quello con il campo contatore di calore minimo
        for (df++; df < &vdf[N_DF]; df++) {         
                if (df->residente ||            // in questo if controlla di saltare frame contenenti entità residenti o vietate 
                    vietato(df, proc, liv, ind_virt))
                        continue;
                if (df->contatore < df_vittima->contatore ||        // tra due entità con contatore uguale si scelga sempre quella di livello minore
                    (df->contatore == df_vittima->contatore &&
                     df_vittima->livello > df->livello))
                        df_vittima = df;
        }
        return df_vittima;
}


// routine delle statistiche
/* può essere chiamata ogni volta che si vogliono aggiornare le statistiche di utilizzo in base ai valori correnti dei bit A nei descrittori di tabelle e pagine virtuali. Poichè la funzione, in particolare la invalida_TLB, è molto costosa ,abbiamo scelto di chiamarla solo quando si è verificato un page fault.  
L'algoritmo utilizzato per l'aggiornamento del contatore serve a implementare una approssimazione LRU per la scelta della vittima. L'idea è di usare il contatore come un registro a scorrimento, in cui ogni posizione rappresenta una chiamata della funzione stat(), dalla più recente nel bit più significativo alla meno recente nel bit meno significativo.
Ogni bit contiene il bit A visto dalla corrispondente stat(). in questo modo il contatore mantiene la storia degli ultimi 32 bit A visti, dando maggior peso a quelli più recenti. La pagina che ha il contatore minimo è quella che non ha visto accessi da più tempo
*/

void stat()
{
        des_frame *df1, *df2;
        faddr f1, f2;
        bool bitA;
        // con questo for scorre tutti i descrittori di frame alla ricerca di quelli che contengono tabelle
        for (natq i = 0; i < N_DF; i++) {
                df1 = &vdf[i];
                if (df1->livello < 1)       // se il livello è minore di 1 il frame o è vuoto o contiene una pagina virtuale
                        continue;
                f1 = indirizzo_frame(df1);      // una volta trovata una tabella, ne ottiene l'indirizzo fisico
                // e con questo for esamina tutti i suoi 512 descrittori                
                for (int j = 0; j < 512; j++) {
                        tab_entry& des = get_entry(f1, j);
                        //  controlla che i descrittori puntino ad entità presenti altrimenti la funzione fa la invalida_TLB                       
                        if (!extr_P(des))
                                continue;
                        bitA = extr_A(des);         // estrae il bit A di ogni descrittore    
                        set_A(des, false);          //azzera il bit A ottenuto
                        // ottiene un puntatore dal descrittore del frame che contiene l'entità puntata, eventualmente saltando quelle residenti                        
                        f2 = extr_IND_FISICO(des);
                        df2 = descrittore_frame(f2);
                        if (!df2 || df2->residente)     // di quelle residenti non importa aggiornare il contatore
                                continue;
                        df2->contatore >>= 1;       
                        // aggiorna il contatore in base al bit A
                        if (bitA)
                                df2->contatore |= 0x80000000;
                }
        }
        invalida_TLB();     // a questo punto, avendo azzerato tutti i bit A, dobbiamo invalidare il TLB, in modo che i successivi accessi possano riportare gli opportuni bit A ad 1.
}

// Elenco delle funzioni di utilità 

/*** PAGINE FISICHE ***/

// dato un indirizzo fisico indirizzo_frame restituisce un puntatore al descrittore del frame corrispondente
des_frame* descrittore_frame(addr indirizzo_frame)

// dato un puntatore ad un descrittore di frame df restituisce l'indirizzo fisico (del primo byte) del frame corrispondente
addr indirizzo_frame(des_frame *df)

// restituisce un puntatore al descrittore di un frame libero, se ve ne sono, e zero altrimenti
des_frame* alloca_frame_libero()

// rende di nuovo livero il frame puntato da df
void rilascia_frame(des_frame *ppf)

/*** DESCRITTORI DI TABELLE E PAGINE VIRTUALI ***/

// restituisce un riferimento all'entrata index della tabella (di qualunque livello) di indirizzo tab
tab_entry& get_entry(addr tab, natl index)

// restituisce un riferimento al descrittore di livello liv da cui passa la traduzione dell'indirizzo ind_virt nello spazio di indirizzamento del processo proc
tab_entry& get_des(natl proc, int liv, add ind_virt)

// estrae il bit P da descrittore
bool extr_P(tab_entry descrittore)

// estrae il bit A da descrittore
bool extr_A(tab_entry descrittore)

// estrae il bit D da descrittore
bool extr_D(tab_entry descrittore)

// estrae il campo indirizzo fisico da descrittore (il campo + significativo se il bit P è ad 1)
addr extr_IND_FISICO(tab_entry descrittore)

// estrae il campo indirizzo di massa da descrittore (il campo è significativo se il bit P è a 0)
addr extr_IND_MASSA(tab_entry descrittore)

// setta il valore del bit P in descrittore in base al valore di bitP
void set_P((tab_entry& descrittore, boot bitP)

// setta il valore del bit A in descrittore in base al valore di bitA
void set_A((tab_entry& descrittore, boot bitA)

// setta il valore del bit D in descrittore in base al valore di bitD
void set_D((tab_entry& descrittore, boot bitD)

// scrive ind_fisico nel campo indirizzo fisico di descrittore
void set_IND_FISICO(tab_entry& descrittore, addr ind_fisico)

// scrive ind_massa nel campo indirizzo di massa di descrittore
void set_IND_MASSA(tab_entry& descrittore, addr ind_massa)

/*** FUNZIONI DI SUPPORTO ALLA MEMORIA VIRTUALE ***/

// carica dallo swap il contenuto del frame descritto da df in base alle informazioni contenute nel descrittore stesso (legge dallo swap dal blocco df->ind_massa)
void carica(des_frame* df)

// copia il contenuto del frame descrittto da df nel corrispondente blocco dello swap (scrive nel blocco df->ind_massa)
void scarica(des_frame *df)

// rende presente l'entità contenuta in df inzializzando l'opportuno descrittore di tabella o pagina virtuale
void collega(des_frame* df)

// rende non più presente l'entità contenuta in df. Restituisce true se è necessario scaricare l'entità contenuta prima di sovrascriverla
void scollega(des_frame* df)


/******** Fine Memoria Virtuale ********/


/* 
   
    ############################################################################################### 
   #																			                   #
   #					TI SEI MERITATO UN BEL CAFFE', mo fattel tu!                               #
   #																		                       #
    ############################################################################################### 


*/






































