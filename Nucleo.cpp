Vito ciao

/********Dichiarazione assembler di un processo ********/
        .extern c_processo
a_processo: 
        /* salva lo stato del processore nel descrittore del processo identificato dalla variabile esecuzione, salva anche (nel campo constesto[I_RSP] del descrittore di processo) il contenuto del registro RSP come lasciato dal processore dopo il cambio di pila e il successivo salvataggio in questa delle cinque parole lunghe. */
        call salva_stato       
        
        call c_processo         
        
        /* carica nel processore lo stato contenuto nel descrittore del processo identificato da esecuzione, ripristinerà il registro RSP affinchè la iretq finale possa estrarre dalla pila sistema queste informazioni, facendo dunque proseguire il processo dal punto in cui era stato interrotto. */
        call carica_stato       

        iretq 


/******** Coda di processi ********/

struct proc_elem {
        natl id;        // identificatore del processo
        natl precedenza;        // priorità del processo
        proc_elem *puntatore;       // puntatore al prossimo processo in coda
};
proc_elem *esecuzione;      // processo in esecuzione
proc_elem *pronti;      // coda dei processi pronti per essere eseguiti



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

