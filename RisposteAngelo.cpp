/***************************************************************************/
Delle 54 domande (MOLTE SONO DUPLICATI!)  ho risposto 
	alla 1,4,6. del 18/01/2019
	alla 2,5 del  del 02/02/2019
	alla 54 del 30/04/2019
	
	



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

/***************************************** 02/02/2019
**************************************************/

2) Cosa bisogna controllare per capire se una tabella in un frame può essere
rimpiazzabile?

-> il campo bool residente del descrittore di frame; 

/********************************************************************/

5) Spiegare cosa succede quando avviene un cambio di processo. Qual'è
l'eccezione più importante del corso? Spiegare il funzionamento di
carica_stato,salva_stato. A che serve il campo punt_nucleo?Perchè punta in
fondo alla pila sistema.(Ci riferiamo al caso di passaggio da livello utente a livello
sistema) Come viene creato un processo?

-> "la salva_stato" e "la carica_stato" si utilizzano quando vi è un cambio di processo, e quindi quando il processo stesso esegue una INT, se il processore genera un'eccezione, se il
processore accetta un'interruzione esterna. Il codice che si segue è il seguente:

call salva_stato
...
call carica_stato
iretq

la "salva_stato" salva lo stato del processore nel descrittore del processo identificato
dalla variabile esecuzione. Quindi salva anche il contenuto di RSP come lasciato dal processore dopo il cambio di pila e il successivo salvataggio in questa delle cinque parole
lunghe. 
la "carica_stato" ripristina anche questo registro in modo che la iretq possa estrarre dalla pila le 5 parole facendo proseguire il processo dal punto in cui era stato interrotto.

-> "punt_nucleo" nel des_proc punta alla pila sistema di P e si trova all'offset a cui lo legge
l'hardware.

/*************************************** 30/04/2019 *******************************************/
54) Finestra di memoria e perché è ragionevole che fm sia all'inizio della
memoria virtuale ?  (4 Aprile 2017 - pag1/2)

-> viene creata per mantenere la MMU attiva anche quando deve accedere ad M1 (prima si
disattivava perché sennò la routine di page fault poteva accedere soltanto ad
M2). Si divide quindi anche la memoria virtuale in parte privata e parte utente e la
finestra di memoria si fa coincidere con l'inizio della parte sistema della memoria
virtuale, così gli indirizzi si traducono in loro stessi senza produrre effetti e queste
traduzioni vanno da 0 fino all'ultimo indirizzo della memoria fisica (quindi la
finestra di memoria contiene tutta la memoria fisica). Non si genera mai page
fault dato che ha tutti i bit P=1.

-> perché così indirizzi fisici e virtuali coincidono


/***********************************************************************************/

56) Differenza tra interruzioni ed eccezioni, chi le solleva, chi le accetta e cosa fa
il processore in un caso e nell'altro

-> Le interruzioni possono essere ESTERNE o INTERNE (SOFTWARE). Le esterne arrivano alla
CPU tramite i piedini connessi al controllore APIC. Possono giungere in qualsiasi momento
quindi sono asincrone al programma. La CPU esamina eventuali richieste di interruzione
solo dopo la fine di una fase di esecuzione di un'istruzione e l'inizio di un'altra fase
di chiamata, quindi non possono sospendere l'esecuzione di un'istruzione.
Le interruzioni interne sono prodotte da istruzioni del tipo INT $tipo, e vengono sollevate
alla fine dell'esecuzione dell'istruzione stessa, quindi sono sincrone rispetto al programma
in esecuzione e non possono sospendere l'esecuzione di una istruzione.

-> Le ECCEZIONI sono invece prodotte da circuiti interni alla CPU ogni volta che si verifica
una condizione di errore o speciale, mentre la CPU esegue le normale istruzioni.
Possono sospendere dunque l'esecuzione di un'istruzione.
Sono di tipo: TRAP -> sollevate solo tra l'esecuzione di una istruzione e la successiva.
FAULT-> vengono sollevate durante l'esecuzione di una istruzione, in caso di errori recuperabili, causandone la sospensione.
ABORT-> vengono sollevate durante l'esecuzione di un'istruzione, in caso di errori gravi 
e irrecuperabili, causandone la terminazione.

Ricevuto il tipo (nel caso di esterne glielo passa l'APIC sul bus, nel caso di interne
è l'operando della INT, nel caso di eccezioni è implicito). Il tipo è associato ad un'entrata
della IDT in cui la CPU trova l'indirizzo di salto alla routine associata.

/*************************************************************************************/

53) Alcuni casi in cui non si può mettere extern "C"

-> quando si ha a che fare con costrutti che in C non ci sono come per esempio le classi

-> quando sono dichiarate extern "C" , il compilatore C++
assume che le due funzioni seguano le regole di aggancio del linguaggio C: in
questo modo rinunciamo alla possibilità di usare l'overloading per le primitive, ma
nella traduzione i nomi delle funzioni rimangono gli stessi.

/**************************************************************************************/









































