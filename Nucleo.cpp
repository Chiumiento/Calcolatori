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
        faddr cr3;      //
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
des_proc *des_p(natl id);

// sceglie il prossimo persorso da mettere in esecuzione (cambiando quindi il valore della variabile esecuzione)

/******** ********/















