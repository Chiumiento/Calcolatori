/***************************************************************************/
Delle 54 domande (MOLTE SONO DUPLICATI!)  ho risposto 
	alla 1,4,6. del 18/01/2019
	alla 2 del  del 02/02/2019
	



/************************** 18/01/2019 ***********************************************/

1) Quali informazioni ci sono nel descrittore di pagina fisica. A cosa serve
ricordare l'indirizzo virtuale nel registro del processore cr2, durante il page-fault.
A cosa serve ricordarmi l'indirizzo del blocco di memoria di massa?

// avremo un descrittore di frame per ogni frame della parte M2.  Lo scopo del
// descrittore e' di contenere alcune informazioni relative al contenuto del
// frame corrispondente. Tali informazioni servono principalmente a
// facilitare o rendere possibile il rimpiazzamento del contenuto stesso.
struct des_frame {
	int	livello;	// 0=pagina, -1=libera, >0=livello tabella
	bool	residente;	// pagina residente o meno
	// identificatore del processo a cui appartiene l'entita'
	// contenuta nel frame.
	natl	processo;
	natl	contatore;	// contatore per le statistiche
	// blocco da cui l'entita' contenuta nel frame era stata caricata
	natq	ind_massa;
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
		vaddr	ind_virtuale;
		des_frame*	prossimo_libero;
	};
};

-> ind_massa: numero di blocco dell'area di swap da cui l'entità era stata caricata.
Ricordare quale era il blocco da cui era stata caricata ogni pagina virtuale è utile nel caso
in cui, quando la pagina virtuale viene scelta come vittima, si scopre che non è stata modificata (D = 0 nel descrittore di pagina virtuale): allora è possibile riutilizzare
la pagina che si trova già nello Swap, ma ovviamente bisogna sapere dov'è (14 Maggio 2018 - pag8).

-> durante un page-fault quando la routine va in esecuzione, legge dal registro CR2 l'indirizzo che la MMU non è riuscita a tradurre, infatti viene lasciato in quel registro
dalla MMU. Letto quell'indirizzo la routine carica la pagina che lo contiene e tutte
le tabelle necessarie per la traduzione. (13 Maggio 2019 - pag6)

/********************************************************************************************/

4) Come avviene la traduzione tra indirizzo virtuale e indirizzo fisico. Dove si
trovano le tabelle di livello 4 e da chi vengono caricate?Quale compito svolge
il registro cr3? L'offset da 12 bit quali vincoli comporta nella memorizzazione
delle pagine nella memoria? Quando è attiva la memoria virtuale a quale parte, il
software, della memoria fisica può accedere?

-> Al primo passo la MMU deve leggere il corretto descrittore (CR3 punta alla tabella di
livello 4 del processo in esecuzione, le tabelle di livello 4 si trovano nella parte M2
della memoria protetta e vengono caricate dal bootstrap). Sia i4 l'indice di livello 4
(bit 39-47). L'indirizzo a cui vuole leggere è dunque CR3 +i4*8. Letto il descrittore 
di livello 4, la MMU ne estrae il campo indirizzo (bit 12-51), lo concatena con i bit 30-38
di V (indirizzo virtuale) e con altri 3 bit a 0, ottendendo cosi l'indirizzo del descrittore
di livello 3. E cosi via per i descrittori di livello 2 e 1. Arrivata al livello 1, la MMU 
estrae il campo F (12-51), lo concatena con l'offset di V (bit 0-11) e ottiene cosi la
traduzione. (6 Maggio 2019 - pag8).
-> L'offset dei 12 bit credo comporti che le pagine debbano essere grandi almeno 4KiB.

/*******************************************************************************************/
6) Spiegare come è fatto un handler. Guardando il codice quanti handler abbiamo,uno solo o molti? Una volta caricato il processo esterno si può fare delle
predizioni sui punti dove ripartirà? Perchè sono due i punti su cui può ripartire il
processo esterno? Che compito svolge la EOI in tutto ciò?

-> l'handler ha il solo scopo di mandare in esecuzione un processo, il quale si preoccupa
di svolgere le operazioni che prima erano svolte dal driver.
Abbiamo molti handler, non solo uno:
	handler_i:
		call salva_stato
		call inspronti
		movq $i,%rcx
		movq a_p(, %rcx,8),%rax
		movq %rax,esecuzione
		call carica_stato
		iretq
Per ogni possbile interruzione il modulo sistema deve predisporre un handler che si preoccupa
di mettere in esecuzione il corrispondente processo esterno, prendendo il proc_elem dalla
tabella a_p.

-> Dopo la loro creazione i processi esterni non terminano più, e dopo aver risposto ad una
richiesta di interruzione si limitano a sospendersi in attesa di un'altra. Quindi fanno un ciclo infinito e dopo ogni iterazione del for terminano con wfi(). la primitiva wfi() salva lo stato del processo esterno, invia l'EOI all'APIC e mette in esecuzione un altro processo di fatto sospendendo il processo esterno, che potrà riandare in esecuzione solamente quando il corrispondende handler verrà nuovamente invocato, in risposta ad una nuova richiesta di 
interruzione della stessa interfaccia. SI NOTI che dopo la prima volta che il processo esterno viene avviato, la coppia call carica_stato, iretq nel codice dell'handler caricherà
lo stato salvato dall wfi(). Infatti se l'handler è in esecuzione vuol dire che l'interfaccia
ha inviato una nuova richiesta che l'APIC ha fatto passare e dunque esso stesso deve aver
ricevuto una EOI per la richiesta precendente, e dunque l'ultima cosa che il processo esterno
aveva fatto in precendenza era proprio chiamare la wfi().

/***************************************** 02/02/2019 **************************************************/

2) Cosa bisogna controllare per capire se una tabella in un frame può essere
rimpiazzabile?

-> il campo bool residente del descrittore di frame; 

/********************************************************************/





















