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

