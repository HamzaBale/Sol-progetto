#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
typedef struct configfile {
    int k; //Number of supermarket checkouts
    int c; //Number of max customers in the supermarket
    int e; //Number of customers that need to exit before make enter other E customers
    int t; //Min time for customers to buy
    int p; //Number of max products for each customers
    int s; //Seconds to do a product by the cashiers
    int s1; //If the are S1 queues with <=1 customers -> Close one of them
    int s2; //If the is a queue with more then S2 customers in queue -> Open another    supermarket checkouts
    int directornotify; //time for notifying director
    int ncheckop; //How much checkout are opened 
}config;


typedef struct checkout{

   int id;  //  1 < id < K num max of checkouts K
   int fixedtime; // random time for 1 product to be executed
   int nclients; //numero clienti in cassa
   int nproducts; //numero prodotti totali in cassa
   int nclose;
   float timeofservice; // time of service 
   int opened;   
}checkout;


static config* cf;
static int sock;
static int* clienticassa;
static int* casseaperte;
static int nclienti_attesa; // POSSO CONTARE QUELLI CHE DEVONO USCIRE ASPETTARE CHE SI SOMMINO E POI MANDO MESS A DIRETTORE CHE DA PERMESSO PER APRIRE.
static int numcasse;

volatile sig_atomic_t sighup=0;

volatile sig_atomic_t sigquit=0;

static int uscire=0;
static int max;
static int indice=0;
static int chiudi=0;
static int apri=0;
static int allexit=0;
static int close2=0;
static int modified = 0;

pthread_mutex_t indicemutex;
pthread_mutex_t clienticassamutex;
pthread_mutex_t cassamutex;


config* configset(FILE* readFile,const char *argv[]);


void handler(int num){

if(num == 1) sighup=1;
if(num == 3) sigquit=1;
send(1,"Ricevuto Segnale\n",20,0);

}

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

/*void* director_customer(void *arg){ //create Cf->c CLIENTI, TIENE CONTO DI QUELLI ATTIVI(penso per ora P=0) (LA GESTIONE DEI CLIENTI VIENE FATTA DAL SUPERMERCATO)

	int maxcustomers=0;
	int activecustomers=0;
	pthread_t* customers;
	

	
	int i=0;
	
	maxcustomers=cf->c;
	activecustomers=maxcustomers;
	
	int numberofcheckout=cf->k;
	
	int Tchecks=cf->e;
	
	
	checkout=malloc(numberofcheckout*sizeof(pthread_t));
	
	customers=malloc(maxcustomers*sizeof(pthread_t));
	
	
	for(i=0;i<(maxcustomers);i++) { //all'inizio i clienti entrano tutti C. //VIENE FATTA DAL SUPERMERCATO.
		if (pthread_create(&customers[i],NULL,customers,(void*) (intptr_t) i)!=0) {
            fprintf(stderr,"fail while creating customer n %d",i);
            pthread_exit(NULL);
	}}
	
	
	while(activecustomers > 0){
		
		while(activecustomers > 0){
		
		pthread_mutex_lock(customervar);
        //customervar=1; 
        pthread_mutex_lock(cusdirector);
		while(customervar == 0)  pthread_cond_wait(cusconddirector,cusdirector);  //aspetto che  qualche cliente mi svegli cosi esco serve per risvegliare il direttore in caso unbo volesse uscire
		pthread_mutex_unlock(cusdirector);
		pthread_mutex_unlock(customervar);
	
			
			
			 pthread_mutex_lock(&customercondlockdirector);
			 
             pthread_mutex_lock(&customerlockdirector);
             quitsupermarket=1;
             pthread_mutex_unlock(&customerlockdirector);
            
             pthread_cond_broadcast(customerconddirector);//risveglio clienti. TUTTI I CLIENTI CHE VOGLIONO USCIRE. (CON IL PROCESSO MI BASTA MANDARE UN MESSAGGIO AL SUPERMERCATO E LUI SVEGLERà TUTTI I CLIENTI.
              //l'unica cosa che deve fare il direttore è controllare se esiste qualcuno che vuole uscire. esiste? alloro scrivo a supermcato di svegliarli 
              //si addormentano su var in supermercato, per leggere il valore di customervar(num clienti che escono) basta farmela inviare dopo un messaggio.
            

             pthread_mutex_unlock(&customercondlockdirector);
             
             
			pthread_mutex_lock(&customervar);
			customervar=0;
		    pthread_mutex_lock(&customervar); 
		    
		    pthread_mutex_lock(&customerlockdirector);
            quitsupermarket=0;
            pthread_mutex_unlock(&customerlockdirector);
		
	
	
	 if(activecustomers==0){
		
		    for ( i=0;i<cf->k;i++) {
		    	
                   pthread_mutex_lock(&check_directorcond_lock[i]);
                    exitbroadcast=1;
                    pthread_cond_signal(checkdirectorcond[i]);
                   pthread_mutex_unlock(&check_directorcond_lock[i]);
               } 
		
		
	}
	
	
	if(activecustomers == (cf->c-cf->e) && sighup!=1 && sigquit!=1) break;
	
	
	}
	
	
		if(activecustomers == (cf->c-cf->e) && sighup!=1 && sigquit!=1){
			maxcustomers=maxcustomers+(cf->e);
			int j=cf->c;
			
			if((customers=realloc(customers,maxcustomers*sizeof(pthread_t)))==NULL) {
                fprintf(stderr,"realloc fail");
            }
			
			 for(i=j;i<maxcustomers;i++){
               	if (pthread_create(&customers[i],NULL,customers,(void*) (intptr_t) i)!=0) {
                    fprintf(stderr,"fail while creating customer n %d",i);
                
                }
           
			activecustomers++;	
		}
	
	
	}
	
	
	
	
	
	
	
	
	
	
	

	
	
}



for (i=0;i<maxcustomers;i++){ //attendo la fine di tutti i thread che ho creato (clienti).
        if (pthread_join(customers[i],NULL) == -1 ) {
            fprintf(stderr,"DirectorSMControl: thread join, failed!");
        }
    }
	
	
	
	
	
	
	
	
	
	
	
	   pthread_exit (NULL); 
}

*/

/*
	
void* checkdirector(void *arg){ 
	
	int numberofcheckout=0;
	int Tchecks=0;
	int i=0;
	int j=0;
	numberofcheckout = cf->k;
	Tchecks=cf->e;
	int out=0;
	pthread_t* checkout;
	
/*	checkout=malloc(numberofcheckout*sizeof(pthread_t)); // CREA I THREAD CASSA IL SUPERMECATO.
	
		for(i=0;i<(Tchecks);i++) {  // INIZIALLIZZA I THREAD CASSA( SUPERMERCATO)
	pthread_mutex_lock(&qopenmutex); 
	qopen[i]=1;
	pthread_mutex_unlock(&qopenmutex); 
	pthread_mutex_lock(&checkarraymutex);	
	checkarray[i].opened=1;	
	pthread_mutex_unlock(&checkarraymutex);
	 //thread checkout venogono creati man mano.
		if (pthread_create(&checkout[i],NULL,check,(void*) (intptr_t) i)!=0) {
            fprintf(stderr,"fail while creating checkout n %d",i);
            pthread_exit(NULL);
	}
 
	}	*/
	
	
	
	
	
/*	
	int count =0;
	
	while(sigquit!=1 && sighup != 1){
		
		
		for(i=0;i < Tchecks; i++){ //CONTO LE CASSE APERTE(SUPEMERCATO), QUESTE LE CONTO DIRETTAMENTE NEL ARRAY int* clienticassa; (CONTIENE NUM CLIENTI SU CASSE, OVVIAMENTE QUELLI CON -1 SONO CHIUSE)
		// UNICO CHE PUO CHIUDERE LE CASSE è DIRETTORE, QUANDO CHIUDE 1 CASSA AZZERA SOLO VALORE DELL'ARRAY[i] (QUANDO INVIO MESSAGGIO A SUPERMERCATO PER CHIUDERE CASSA, PRIMO CARATTERE SARA i.
		//le cassse aperte in ordine crescente di i
		
		
		
		//posso tenere 2 array, uno per n clienti e uno per casse aperte.
		
		
		
			
			
			
			pthread_mutex_lock(checklength);
			
			if(checklength[i] > 1) {
				count++;
			}
			
			//invio a supermercato stringa con "i".
			
			if(count > cf->s1){
				pthread_mutex_lock(&checkclosedmutex);
				checkclosed[i]=1; //quando chiudo una cassa i clienti gia in cassa finiscono, poi i nuovi clienti basta che controllino se cassa aperta o no.
				pthread_mutex_unlock(&checkclosedmutex);
				pthread_mutex_lock(&qopenmutex); 
              	qopen[i]=0;
	            pthread_mutex_unlock(&qopenmutex); 
	            pthread_mutex_lock(&checkarraymutex);	
	            checkarray[i].opened=1;	
             	pthread_mutex_unlock(&checkarraymutex);
	            break;
			}
			
			
		}
		
			for(i=0;i < Tchecks; i++){
			
			pthread_mutex_lock(&checklength);
			
			if(checklength[i] >= cf->s2) {
				for(j=0;j<numberofcheckout; j++){
				pthread_mutex_lock(&qopenmutex);	
				if(qopen[j] == 0){
				
				pthread_mutex_lock(&checkchiudidmutex);
				checkchiudid[j]=0; 
				pthread_mutex_unlock(&checkchiudidmutex);
				pthread_mutex_lock(&qopenmutex); 
              	qopen[j]=1;
	            pthread_mutex_unlock(&qopenmutex); 
	           	pthread_mutex_lock(&checkarraymutex);	
	            checkarray[j].opened=1;	
             	pthread_mutex_unlock(&checkarraymutex);
	            if (pthread_create(&checkout[j],NULL,check,(void*) (intptr_t) i)!=0) {
                        fprintf(stderr,"fail while creating checkout n %d",i);
                        pthread_exit(NULL);
            	}
	            out=1;
	            break;
			}
			pthread_mutex_unlock(&qopenmutex);
			}
			
			}
			pthread_mutex_unlock(&checklength);
			if(out == 1) break;
			
		}
		

	}
	
	if(sigquit == 1){
		 for ( i=0;i<cf->k;i++) { //svegliare i cassieri che sono in wait sia del cliente sia del direttore
            pthread_mutex_lock(&check_directorcond_lock[i]);
            pthread_cond_signal(checkdirectorcond[i]);
            pthread_mutex_unlock(&check_directorcond_lock[i]);
            pthread_mutex_lock(&check_cond_lock[i]);
            pthread_cond_signal(checkcond[i]);
            pthread_mutex_unlock(&check_cond_lock[i]);
        }  
	}
	
	for(i=0; i< cf->k; i++){
		
		if(qopen[i] == 1) pthread_join(checkout[i],NULL);
		
		
	}
	
	
	
	
    pthread_exit (NULL); 
}
*/

//SERVE 1 THEREAD CHE CICLI E FACCIA USCIRE I CLIENTI (CICLO SU RCV, SE RCV = STRINGA GIUSTA ==> TI FACCIO USCIRE)





/*void* cicla(void *arg){  //chiusura e apertura casse.


 for(int i=0;i<cf->k;i++){
 	
     	if(clienticassa[i] > 1) count++;
     	if(clienticassa[i] > max) max=clienticassa[i];
     	
	 }

}*/




void* sed(void* a){ 
int sockt = *((int *) a);
char *es = "esci";
char *ap="apri";
char *cl="close";
char *sg="sigquit";
char *sh="hup";
int indice = 0;
char in[100]={};
char *b;
while(sigquit!=1 && sighup!=1){
//sprintf(in,"%d",indice);
//printf("chiudi=%d\n",close2);
//b=strcat(in,cl);

//if(close2==0 && apri==0) send(sock,es,strlen(es),0);
send(sock,es,strlen(es),0);
if(close2 == 1){
printf("mandato chiudi\n");
fflush(stdout);
 pthread_mutex_lock(&indicemutex);
 sprintf(in,"%d",indice);
 b=strcat(in,cl);
 pthread_mutex_unlock(&indicemutex);
 send(sockt,b,strlen(b),0);
 close2=0;
}
else if(apri==1){
printf("mandato apri\n");
fflush(stdout);
 pthread_mutex_lock(&indicemutex);
 sprintf(in,"%d",indice);
 b=strcat(in,ap);	
 pthread_mutex_unlock(&indicemutex);
 send(sockt,b,strlen(b),0);
 apri=0;
}
sleep(1);
}
if(sighup==1){
printf("sighup");
fflush(stdout);
send(sock,sh,strlen(sh),0);
printf("HO MANDATO SIGHUP\n");
}
else if(sigquit == 1){
printf("HO MANDATO SIGQUIT\n");
fflush(stdout);
send(sockt,sg,strlen(sg),0);
}
printf("finito\n");
fflush(stdout);

pthread_exit(0);


}




void* control(void* a){ 
int mytime=0;
int max =0;
int count = 0;
int i=0;
int ap=0;

while(sigquit!=1 && sighup != 1){
     count = 0;
     max= 0;
     chiudi=0;
     ap=0;
       
        if(modified == 1){ //metterlo in wait
	pthread_mutex_lock(&clienticassamutex);
	pthread_mutex_lock(&cassamutex);
	for(int i=0;i<cf->k;i++){ 
	
	if(casseaperte[i]!=-1) max = clienticassa[i]; 

	}
	//array di int con indice la cassa e con valore il numero di clienti in cassa.
	for(i=0;i<cf->k;i++){
    if(clienticassa[i] <= 1 && casseaperte[i]!= (-1) ){ 
    count++;}
    if(clienticassa[i] > max && casseaperte[i]!= (-1)) max=clienticassa[i];
	 }
	pthread_mutex_unlock(&cassamutex);
	pthread_mutex_unlock(&clienticassamutex);
	 if(count>= cf->s1 || max >= cf->s2) mytime = 1;
printf("count= %d\n",count);
fflush(stdout);
printf("max= %d\n",max);
fflush(stdout);
if(mytime==1){ //momento in cui il direttore chiude o apre casse.
   
     
    // if(clienticassa[0] != NULL) max = clienticassa[0]; 
     
     /*for(int i=0;i<cf->k;i++){
     	if(clienticassa[i] > 1) count++;
     	
     	if(clienticassa[i] > max) max=clienticassa[i];
	 }*/
     
    if(count >= cf->s1) chiudi = 1;
     
    //if(max > 10) ap = 1;
     
    if(count >= cf->s1 && max >= cf->s2){ chiudi = 0;
    ap=0; }
     printf("%d %d\n",chiudi,ap);
     fflush(stdout);
     pthread_mutex_lock(&cassamutex);
     if(chiudi == 1){
	 i=0;	
	  while(i<cf->k){
	        printf("cassaaperte[%d]= %d\n",i,casseaperte[i]);
	        fflush(stdout);
	  	if(casseaperte[i] == -1)i++;
	  	
	  	else break;
	  }
	  casseaperte[i] = -1;
	  numcasse--;
	  pthread_mutex_lock(&indicemutex);
	  indice=i;
	  pthread_mutex_unlock(&indicemutex);
	  close2=1;
	  	  }

     if(ap == 1){
     printf("ap= %d\n",ap);
     fflush(stdout);
     	i=0;
     while(i<cf->k){
    
	 if(casseaperte[i] != -1)i++;
	 else break;
	  }
	  casseaperte[i]=1;
	  numcasse++;
	  pthread_mutex_lock(&indicemutex);
	  indice=i;
	  pthread_mutex_unlock(&indicemutex);
	  apri=1;
	  }
mytime = 0;
    }
        pthread_mutex_unlock(&cassamutex);
	pthread_mutex_unlock(&clienticassamutex);
	
	modified=0;
}

}
printf("finito\n");
fflush(stdout);


pthread_exit(0);
}





































int main(int argc, char *argv[]){

 ///////////////////////setting config////////////////////
FILE *fileptr; // ptr to file
FILE* directorfile;
FILE *fileport;
cf = configset(fileptr,argv); //setting struct config
printf("args = %d",argc);
for(int i=0;i<argc;i++) printf("argv = %s\n",argv[i]);
fflush(stdout);
int PORT;
char *fileline;
fileline=malloc(30*sizeof(char));
fileport = fopen("superport.txt", "r");


  fgets(fileline,30,fileport);
 printf("CIAO = %d\n",atoi(fileline)); 
 fflush(stdout);  
 PORT=atoi(fileline);
if ((directorfile = fopen("direttore.PID", "w")) == NULL) { 
	fprintf(stderr, "Stats file opening failed");
	exit(EXIT_FAILURE);
    }

fprintf(directorfile,"%d",getpid());
fflush(directorfile);

struct sigaction s;
memset(&s,0,sizeof(s));
s.sa_handler=handler;

if (sigaction(SIGHUP,&s,NULL)==-1) {
        fprintf(stderr,"errore gestore");
    }
    
if (sigaction(SIGQUIT,&s,NULL)==-1) {
        fprintf(stderr,"errore gestore");
    }
   
 
    printf("siguit %d =\n",sigquit);
    fflush(stdout);
///////////////////////////////////////////////////////////
char message[256] = "connessione accettata\n";

char chiuda[25] ="chiudi\n";
char apra[25] ="apra\n";
char esci[25] ="esci\n";
int i=0;
int checklenght[cf->k];

int dirnot[cf->k];
pthread_t control1;

casseaperte=malloc((cf->k)*sizeof(int));

clienticassa = malloc(cf->k*sizeof(int));

for(i=0;i<cf->ncheckop;i++){
 casseaperte[i]=1;
 clienticassa[i]=0;
 }
for( i=cf->ncheckop;i<cf->k;i++) {
casseaperte[i]=-1;
clienticassa[i]= -1 ;
 }


numcasse=cf->ncheckop;
 printf("creation succesuful1\n");
 fflush(stdout);
//ciclo in cui ascolto sulla socket.

    //send(director_socket, esci, sizeof(esci),0); //ogni ciclo invia al supermercato questo messaggio al fine di far uscire tutti i clienti che stanno in una coda.(RISVEGLIA I CLIENTI NELLA WAIT)
    //supermercato fa una broadcast ogni volta che riceve sto messaggio. (main lo fa direttamente).
    
    
    //ricevo indice cassa e numero cassa.
    //thread invia "esci" sleep(2);
    int valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client\n"; 
    //char buffer[10024] = {0}; 
   FILE *fp;
    //clienticassa = malloc(cf->k*sizeof(int));
     pthread_t sd;
  
  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   printf("creation succesuful2\n");
   fflush(stdout);
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   printf("creation succesuful3\n");
   fflush(stdout);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        fflush(stdout);
        return -1; 
    } 
   // write(sock , hello , 20 ); 
    printf("connection succesuful");
     fflush(stdout);
    int *z = malloc(sizeof(*z));          
    *z=sock;
    
 if(pthread_create(&sd,NULL,(void*)sed,(void*)z)!=0) {

fprintf(stderr, "Error creating thread\n");

return 1;

}
 fflush(stdout);


 if(pthread_create(&control1,NULL,(void*)control,(void*)z)!=0) {

fprintf(stderr, "Error creating thread\n");

return 1;

}  

    
 char buffer[8192]; // or whatever you like, but best to keep it large
int count = 0;
int total = 0;
int c =0;
char* es="esci";
int nos=0;
int valore=0;
int valore2=0;
int valore3=0;
int ind=0;

     //while(1){
 fflush(stdout);
      while ((count = recv(sock, &buffer[total], sizeof buffer - count, 0)) > 0 && sigquit!=1 && sighup != 1)
{

    printf("COUNT ==== %d\n",count);
    fflush(stdout);
    c=total;
    total += count;

    for( i=c;i<total;i++){ 
    if(buffer[c] == 'u'){ 
     uscire=1;
    break;
    }
    //printf("%d = %c\n",i,buffer[i]); 
    if(buffer[i] == 'o') ind=i+1;
    }
    //riceve (indice,"i",val,no);
    
    //printf("char =%c\n",buffer[c]);
    
    nos=(buffer[c])-'0';
    
    if(ind == total-1) valore=(buffer[ind])-'0';
    else if(ind == total-2) {
    valore=((buffer[ind]) -'0')*10;  
    valore2=((buffer[ind+1]) -'0'); 
    valore=valore+valore2;
     }
    else if(ind == total-3){ 
    valore=((buffer[ind]) -'0')*10;
    valore2=((buffer[ind+1]) -'0')*100;
    valore3=((buffer[ind+2]) -'0');
    valore=valore+valore2+valore3;
    }
    
    //printf("int = %d\n",nos);
    //printf("valore= %d\n",valore);
    pthread_mutex_lock(&clienticassamutex);
    modified=1;
    clienticassa[nos]=valore;
    pthread_mutex_unlock(&clienticassamutex);
}
if (count == -1)
{
    perror("recv");
}
else if (count == 0)
{   
printf("cunt = 0");
fflush(stdout);

sigquit=1;
}

 printf("uscire");
  int true = 1;
setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
 fflush(stdout);
 sleep(1);
 pthread_join(sd,NULL);
setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
  printf("uscire2");
  fflush(stdout);
   pthread_join(control1,NULL);
   
  printf("uscire3");
  printf("\n SIGQUIT =%d",sigquit);
  printf("\n SIGHUP =%d",sighup);
  fflush(stdout);
  printf("DIRETTORE HA FINITO E SI SPEGNE\n");
   fflush(stdout);
   
    pthread_exit(NULL); 
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //prima leggo recv, vedo se è una notifica. se lo è leggo la notifica altrimenti apro cassa e chiudo.

   //	ris = recv(director_socket, &supermercato ,sizeof(supermecato)); 
	
	/*printf("supermecato ha scritto : %s\n",supermecato);
	
	if(ris!=0 && ris!= -1){
	
	if(strcmp(supermercato,"notifica")){
	
	recv(director_socket, &checklenght ,sizeof(checklenght)); //legge i due array di int, e modifico array clienticassa.
	recv(director_socket, &dirnot ,sizeof(dirnot));
	
	for(i=0;i<cf->k;i++){
		
		if(dirnot[i] == 1) clienticassa[i]=checklenght[i];
	
	}
	
	
   }
	
}



	for(int i=0;i<cf->k;i++){
     	if(clienticassa[i] > 1) count++;
     	
     	if(clienticassa[i] > max) max=clienticassa[i];
	 }	
	
if(count>2 || max>10) mytime = 1;


if(mytime=1){ //momento in cui il direttore chiude o apre casse.
     /*int count = 0;
     int max= 0;
     
     if(clienticassa[0] != NULL) max = clienticassa[0]; 
     
     for(int i=0;i<cf->k;i++){
     	if(clienticassa[i] > 1) count++;
     	
     	if(clienticassa[i] > max) max=clienticassa[i];
	 }
     */
  /*   if(count>2) chiudi = 1;
     
     if(max > 10) chiudi = 2;
     
     if(count>2 && max>10) chiudi = 0;
     
     
     
     if(chiudi == 1){	
	  send(director_socket, chiuda, sizeof(chiuda),0);//supermercato chiude una cassa.
	  casseaperte[i] =-1;
	  numcasse--;
	  	  }

     if(chiudi == 2){
	  send(director_socket, apra, sizeof(apra),0); //supermercato apre una cassa.
	  casseaperte[i]=1;
	  numcasse++;
	  }
mytime = 0;
    }
    
       
 

}*/








/*

return 0;
}*/


config* configset(FILE* readFile,const char *argv[]){

char *fileline;
config* conf;
conf=malloc(sizeof(config));
printf("s",conf);
//if (strcmp(argv[1],"testfile/config.txt")==0){
    readFile = fopen("config.txt","r");
    //}
/*else if (strcmp(argv[1],"testfile/config1.txt")==0){
    readFile = fopen("config1.txt","r");
    }*/
    
fileline=malloc(30*sizeof(char));
printf("%s",readFile);
if(readFile){
printf("s",readFile);
 int i=0;
 int j=0;
 char *cpy;
 int b=0;
 printf("FILE LINE = %s \n", fileline);
  while(!feof(readFile)){
                        if(fgets(fileline,30,readFile) != NULL){
                        printf("FILE LINE DIRETTORE = %s \n", fileline);
                        fflush(stdout);
                        i=0;
                        cpy=fileline;
                        while(*fileline != '='){ fileline++;
                        i++;}
                        fileline++;
                       
                        
                       
                        
                        switch(j){
                        
                        case 0: conf->k=atoi(fileline);
                       
                        
                        break;
                        
                        case 1: conf->c=atoi(fileline);
                        break;
                
                        case 2: conf->e=atoi(fileline);
                        break;
                        
                        case 3: conf->t=atoi(fileline);
                        break;
            
                        case 4: conf->p=atoi(fileline);
                        break;
            
                        case 5: conf->s=atoi(fileline);
                        break;
            
                        case 6: conf->s1=atoi(fileline);
                        break;
            
                        case 7: conf->s2=atoi(fileline);
                        break;
            
                        case 8: conf->directornotify=atoi(fileline);
                        break;
            
                        case 9: conf->ncheckop=atoi(fileline);
                        break;
                        
                        default:
                        break;
                        
                        }
                        j++;

                        fileline=cpy;
                        }
                        

              
                             }


}

fclose(readFile);
return conf;
}


void setcheck(checkout* check, int i){
  
  check->id = i+1;
  check->fixedtime=0;
  check->nclients = 0;
  check->nproducts = 0;
  check->timeofservice=0;
  check->opened=0;
}

