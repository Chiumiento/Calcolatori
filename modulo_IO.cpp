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

//ATTENZIONE-> NON USIAMO LA SALVA_STATO E LA CARICA_STATO PERCHE' TANTO SARANNO LE PRIMITIVE SEMAFORICHE A BLOCCARE SALVANDO E CARICANDO LO STATO

/*************************************************************************************************************************************************/

// DRIVER CHE VA IN ESECUZIONE PER EFFETTO DI UNA RICHIESTA DI INTERRUZIONE DA PARTE DELL'INTERFACCIA
// GIRA A LIVELLO SISTEMA E DUNQUE HA BISOGNO DI UNA IRETQ PER TORNARE A LIVELLO UTENTE
.extern c_driver
a_driver_i:
	call salva_stato
	movq $i,%rdi
	call c_driver
	call apic_send_EOI
	call carica_stato
	iretq
 

























