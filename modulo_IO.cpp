//tipica primitiva di lettura nello spazio di IO
// id -> periferica da cui si vuole leggere
// buf -> buffer in cui l'utente vuole  ricevere i dati
// quanti -> numero di byte che si vuole leggere
extern "C" void read_n(natl id,natb* buf,natl quanti);

//tipica primitiva di scrittura
extern "C" void write_n(natl id, const natb* buf,natl quanti);
// const -> la primitiva si limita a leggere il buf, non si può modificare

//un processo P1 incova la read_n
.global read_n
read_n:
	int $IO_TIPO_RN
	ret

.extern c_read_n
a_read_n:
	cavallo_di_troia %rsi
	cavallo_di_troia2 %rsi,%rdx
	call c_read_n
	iretq

//la macro cavallo_di_troia, controlla eventuali indirizzi pericolosi passati dal modulo utente al sistema in quanto
//le primitive girano a livello sistema

//descrittore di periferica, da dove la primitiva legge i dati da utilizzare
struct des_io{
	natw iRBR,iCTL;
	nat* buf;
	natl quanti;
	natl mutex; //garantisce la mutua esclusione tra i processi che vogliono utilizzare la periferica di id = id;
	natl sync; //ne garantisce la sincronizzazione
};

//sarà dunque sufficiente avere un array di questi descrittori e utilizzare id come indice al suo interno

//parte C++ primitica c_read_n
extern "C" void c_read_n(natl id,natb* buf,natl quanti){
	des_io* d = &array_des_io[id];
	
	sem_wait(d->mutex); //solo un processo alla volta puo eseguire le righe 43-46
	d->buf = buf; //da qui legge il driver
	d->quanti = quanti; //da qui legge il driver
	outputb(1,d->iCTL); //abilito le interruzioni
	sem_wait(d->sync); //blocca il processo qui, verrà liberato al termine dell'operazione di IO
	sem_signal(d->mutex); //risveglio eventuali processi bloccati sul semaforo di mutua esclusione
}

/*************************************************************************************************************************************************/

//ATTENZIONE-> NELLE SCRITTURE ASSEMBLER DELLE PRIMITIVE NON USIAMO LA SALVA_STATO E LA CARICA_STATO PERCHE' TANTO SARANNO LE PRIMITIVE SEMAFORICHE A BLOCCARE SALVANDO E CARICANDO LO STATO

/*************************************************************************************************************************************************/

// DRIVER CHE VA IN ESECUZIONE PER EFFETTO DI UNA RICHIESTA DI INTERRUZIONE DA PARTE DELL'INTERFACCIA
// GIRA A LIVELLO SISTEMA E DUNQUE HA BISOGNO DI UNA IRETQ PER TORNARE A LIVELLO UTENTE
/* Ipotizzando che quando arriva l'interruzione in esecuzione ci sia il processo P2, il driver salva e ripristina lo stato di P2 perchè può dover
cambiare il processo in esecuzione, infatti quando l'ultimo byte è stato trasferito il driver deve risvegliare P1 che era fermo sulla sem_signal(d->sync). Quindi
se P1 ha priorità maggiore di P2, è P1 che deve andare in esecuzione mentre P2 in coda pronti. Quindi la carica stato caricherà lo stato o di P1 o di P2 */
.extern c_driver
a_driver_i:
	call salva_stato //
	movq $i,%rdi
	call c_driver
	call apic_send_EOI
	call carica_stato
	iretq

//C++ del driver, che ha lo scopo di leggere il nuovo byte dall'interfaccia e copiarlo nel buffer dell'utente, righe 81-82
/* SI NOTI CHE:
	1) la disabilitazione delle interruzioni è eseguita PRIMA della lettura del byte
	2) invece di chiamare sem_signal(), chiamo direttamente c_sem_signal()
	3) la scrittura in d->buf è eseguita mentre è attiva la memoria virtuale di P2, anche se il buffer era stato allocato da P1
	
1-> la lettura del byte fa da risposta alla richiesta di interruzione da parte dell'interfaccia, che a quel punto può generarne un'altra se
ha un nuovo byte disponibile, quindi se leggiamo l'ultimo byte mentre l'interfaccia ha le interruzioni abilitate, può succedere che l'interfaccia
generi una nuova interruzione facendo partire il driver anche se effettivamente nessuno ha richiesto una lettura.

2-> la sem_signal() salva e ripristina lo stato ma il driver non è un processo e non ha un suo descrittore di processo. Infatti se il c_driver chiamasse
la sem_signal() salverebbe lo stato nel descrittore del processo attivo, cioè P2, e per di piu sovrascriverebbe lo stato della a_driver_i senza 
prima chiamarne una carica_stato. Si noti che chiamando c_sem_signal il driver manipola le code dei processi, e dunque deve essere eseguito con le
interruzioni disabilitate. Questo comporta che anche le richieste di interruzione a precedenza maggiore dovranno attendere che il driver termini prima di
poter essere gestite.*/
extern "C" void c_driver(natl id){
	des_io* d = &array_des_io[id];
	char c;
	
	d->quanti--;
	if(d->quanti == 0){
		outputb(0,d->iCTL);
		c_sem_signal(d->sync);
	}
	
	inputb(d->iRBR,c);
	*d->buf = c;
	d->buf++;
}































