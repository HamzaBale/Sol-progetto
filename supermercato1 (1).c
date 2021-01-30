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





typedef struct checkout{

   int id;  //  1 < id < K num max of checkouts K
   int fixedtime; // random time for 1 product to be executed
   int nclients; //numero clienti in cassa
   int nproducts; //numero prodotti totali in cassa
   int nclose;
   float timeofservice; // time of service 
   int opened;   
}checkout;

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


typedef struct lista{

struct listacustomer *head;

}lista;


typedef struct customer{
	
 int T;
 int id; 
 int P;	
 int myturn;
 int exit;
 int cassa;
}customer;


typedef struct listacustomer{

struct customer *val;
struct listacustomer* next;

}listacustomer;


/*typedef struct listacassa{

struct lista* next;
struct lista* prev;

}lista;*/

















/////set config/////
config* configset(FILE* readFile,char const *argv[]);

int checkcon(config cf);//controlla se i valori sono giusti per il testo del progetto

static config* cf;

/////end set config/////

//////set checkout/////
void setcheck(checkout* check, int i);
//////end set checkout/////
static int new_socket;
static customer** array_customer;
static int* checkclosed;// if checkclosed[id] is 0 don-t close else close that checkout.(create a mutex for it)
static int* checklength;// length of checkout(mutex for it) can be seen by customers and modified by thread of gestione delle queue;
static int* directorqueue;// if queue[id] is 1 send statistics about checklength.
static int uscire=0;
static int * directorsnotify;//queue[id]=1, queuelength sent by cashier id.
static int Tchecks=0;
static checkout* checkarray;//creato dal direttore, alla creazione del thread.

static int * customersq;//array che sta in checkout e tiene conto delle persone in fila (cassa[id]=ha lista di elementi(elemento = nprodotti)

static lista* cassa;
static listacustomer* direttore;

static int quitsupermarket=0;
static int exitbroadcast=0;
//static atomic_int *customers_id=0;

static listacustomer *last;

static listacustomer *list;

static int activecustomers=0;

static int sigquit=0, sighup=0;

static int Allexit=0;
static int done=0;
static FILE* statsfile;
static FILE* directorfile;
static int ind=0;
static int dirnot=0;

static int* qopen;

void Createqueue();
void Freequeue();

static  pthread_mutex_t customerlockdirector  ;

static pthread_mutex_t checkclosedmutex;

static pthread_mutex_t checklengthmutex;

static pthread_mutex_t customersqmutex;

static pthread_mutex_t directorsnotifymutex;

static pthread_mutex_t cassamutex;

static pthread_mutex_t customerqmutex;

static pthread_mutex_t qopenmutex;

static pthread_mutex_t checkarraymutex;

static pthread_mutex_t* lock_cassa;

static pthread_mutex_t cusdirector;

static pthread_mutex_t customervar;

static pthread_mutex_t file_lock;

static pthread_mutex_t customercondlockdirector;

static pthread_mutex_t indice_mut;

static pthread_mutex_t dirnotmutex;
//static pthread_mutex_t* check_directorcond_lock;

///////variabili di condizione//////////////////////////////


static pthread_mutex_t directorupdatelock;
static pthread_cond_t directorupdatecond ; //var condizione su qui viene svegliato il direttore quando directorsnotify[id]=1;

static pthread_cond_t* customercond;
static pthread_mutex_t* customerlock;

static pthread_cond_t* checkdirectorcond;
static pthread_mutex_t* check_directorcond_lock;
static pthread_mutex_t lista_mutex;

static pthread_mutex_t* check_cond_lock;
static pthread_cond_t* checkcond;//cond su cui viene risvwegliato checkout dal cliente (lista di cond)

static pthread_cond_t cusconddirector;

static pthread_cond_t customerconddirector;

//static pthread_cond_t checkdir = PTHREAD_COND_INITIALIZER;
//static pthread_mutex_t  checkdirlock = PTHREAD_COND_INITIALIZER;

pthread_t* check_thread;

///////variabili di condizione//////////////////////////////
#define NUM_MAX_CUSTOMERS 15
#define NUM_MAX_CUSTOMERS_DIRECTOR 100










void *check(void *arg){ //prendere customers metterli nella lista, creare lista cassa[id]=vettore di interi    //devo gestire sigquit e sighup
struct timespec start_time1;

clock_gettime(CLOCK_REALTIME, &start_time1);

struct timespec current_time1;

int identif=(int) (intptr_t) (arg);
int exit1=0;
int id=identif;
int dimcassa=cf->k;//deve stare nel direttore
long randtime=0;
int numeroprodotti=0;
int numeroclienti=0;
int nchiusure=0;
int exittime=0;
checkout check = checkarray[id];
//printf("CASSA CREATA=%d\n",id);

listacustomer *first=NULL; 
listacustomer *temp=NULL; 
customer* cs=NULL;
struct timespec current_time;
struct timespec last_time;
int timer=0;
// I THREAD VENGONO ACCESI DAL DIRETTORE 
while(sigquit!=1 && exitbroadcast == 0){
pthread_mutex_lock(&checkclosedmutex); 
int quit=checkclosed[id];
pthread_mutex_unlock(&checkclosedmutex); 






pthread_mutex_lock(&check_directorcond_lock[id]);//cond per ogni cassa

while(quit!=0){ printf("sono in wait\n");
fflush(stdout);
                pthread_cond_wait(&checkdirectorcond[id],&check_directorcond_lock[id]); //se cassa chiusa ==> wait su cond affinchè non verrà risvegliato dal direttore 
                pthread_mutex_lock(&checkclosedmutex); 
                quit=checkclosed[id];
                pthread_mutex_unlock(&checkclosedmutex); 
                } 
pthread_mutex_unlock(&check_directorcond_lock[id]);
           
    
pthread_mutex_unlock(&checkclosedmutex); 
 
pthread_mutex_lock(&checkclosedmutex);
if(checkclosed[id] != 0) exit1=1;
pthread_mutex_unlock(&checkclosedmutex);


while(sigquit!=1 && exit1 == 0 && exitbroadcast == 0){

pthread_mutex_lock(&checkclosedmutex);

if(checkclosed[id] != 0){
 exit1=1;
break;
}
pthread_mutex_unlock(&checkclosedmutex);

clock_gettime(CLOCK_REALTIME, &current_time);


pthread_mutex_lock(&lock_cassa[id]);   
while(cassa[id].head!= NULL && sigquit!=1){ //sigquit e sig ricordarsi.   //non devo controllare checkclosed perchè devo finire tutti i clienti prima di chiudere, i clienti capiscono dove andare guardando l'array delle casse cassa[id] = dimensione cassa.
pthread_mutex_unlock(&lock_cassa[id]);   
pthread_mutex_lock(&lock_cassa[id]);   
           
                 if(cassa[id].head!=NULL){
                 pthread_mutex_unlock(&lock_cassa[id]);
                 pthread_mutex_lock(&lock_cassa[id]);
                 first=cassa[id].head;//cassas[id] lista di customers. (array di lista di customers struct)
                 
                 //if((first->val)->cassa == id){
                 temp=first;
                 
              
                 checklength[id]--;//modifico valore della Q lenght.
              
                 cs=first->val;
                 pthread_mutex_unlock(&lock_cassa[id]);
                 first=first->next;
       
                //pthread_mutex_unlock(customerqmutex);
                 
                 randtime=(check.fixedtime)+((cf->s)*(cs->P));
                 numeroprodotti+=cs->P;   
                 numeroclienti++;
           
                 cassa[id].head=first;
                 struct timespec t={(randtime/1000),((randtime%1000)*1000000)};
                 nanosleep(&t,NULL);//processa i prodotti del customer. devo segnalare al customer che il cashier ha finito
                 
                 (cs)->myturn=1;
                 clock_gettime(CLOCK_REALTIME, &last_time);
                 timer=last_time.tv_sec-current_time.tv_sec;
                 //printf("cliente fatto\n");
                 
                 pthread_mutex_lock(&customerlock[id]);
                 pthread_cond_broadcast(&customercond[id]);
                 pthread_mutex_unlock(&customerlock[id]);

                 if(timer >= cf->s){ 
              pthread_mutex_lock(&directorsnotifymutex);
              directorsnotify[id]=1;
              pthread_mutex_unlock(&directorsnotifymutex);
              clock_gettime(CLOCK_REALTIME, &current_time);
               }
              
                // }
               /*else  {
               
               first=first->next;
               cassa[id].head = first;
               }*/
                  }
       pthread_mutex_unlock(&lock_cassa[id]);            
                
               //pthread_mutex_unlock(customerqmutex);
               


}

pthread_mutex_unlock(&lock_cassa[id]);
sleep(1);

}


sleep(1);
pthread_mutex_unlock(&checkclosedmutex);

clock_gettime(CLOCK_REALTIME, &current_time1);

pthread_mutex_lock(&checkarraymutex);

checkarray[id].nproducts+=numeroprodotti; //array di casse che tiene conto delle statistiche redatte dalla cassa, e viene usato anche per sapere se una cassa è aperta o chiusa dal direttore
checkarray[id].nclients+=numeroclienti;
checkarray[id].nclose++;
checkarray[id].timeofservice+=current_time1.tv_sec-start_time1.tv_sec;
checkarray[id].opened=0;
printf("%d %d %d BROOO\n",sigquit,exit1,exitbroadcast);
fflush(stdout);
printf("I RISULTATI %d %d %d %d opned = %d\n",id,checkarray[id].nclients,checkarray[id].nclose,checkarray[id].timeofservice,checkarray[id].opened);
fflush(stdout);
pthread_mutex_unlock(&checkarraymutex);
/*
first= cassa[id].head;
while(activecustomers > 0){
cs=first->val;
cs->myturn=1;
if(first!=NULL)printf("ORA SI INIZIA %d\n",(cs)->id);
fflush(stdout); 
 sleep(2);
               
                pthread_mutex_lock(&customerlock[id]);
                 pthread_cond_broadcast(&customercond[id]);
                 pthread_mutex_unlock(&customerlock[id]);
                first=first->next;
                 }*/


}
Tchecks--;
pthread_exit(NULL);
}



void* dirnotify(void *arg){

	int i=0;
	int j=0;
        while(sigquit!=1 && sighup!=1){
        if(dirnot==0){
        for(i=0;i<cf->k;i++){ 
          pthread_mutex_lock(&directorsnotifymutex);
       
        if(directorsnotify[i] == 1){ 
        //printf("dirnotify = %d\n",directorsnotify[i]);
        pthread_mutex_lock(&indice_mut);
        ind=i;
        j=i+1;
        pthread_mutex_unlock(&indice_mut);
        dirnot=1;
        pthread_mutex_unlock(&directorsnotifymutex);
          }
       pthread_mutex_unlock(&directorsnotifymutex);
         }
       
         }
          
    }

pthread_exit(NULL);
	}












void *customers(void *arg){ //serve thread che controlli le casse e cambi quando trova una vuota
	int ciao=0;
	int id=(int) (intptr_t) (arg);
	int customerid=id;
	int i=0; 
	int dim = cf->k;
	int exit1 = 0;
	int j=0;
	int contatore=0;
	long total_time=0;
	long q_time=0;
        int min=0;
	int min2=0;
	int lenght=0;


	struct timespec start_time;
	struct timespec current_time;
	struct timespec start_time2;
	struct timespec current_time2;
	pthread_t thr;

    clock_gettime(CLOCK_REALTIME, &start_time);
 
        unsigned int seed=customerid;
	customer* cliente = malloc(sizeof(customer));
	//printf("customer thread succefully created = %d \n",customerid);

	cliente->id=customerid;
	cliente->P=rand_r(&seed)%(cf->p);
	cliente->T=rand_r(&seed)%(cf->t);
	cliente->myturn = 0;
	cliente->cassa=0;
	cliente->exit=0;
	listacustomer* elem; 
	listacustomer* t; 
	listacustomer* head = malloc(sizeof(listacustomer));
	head->val=cliente;
	if(sighup != 0 || sigquit != 0) {
	      printf("cliente spento per segnale \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
	
	if(cliente->P!=0 && sigquit==0 && sighup!= 1){	
	if(sigquit != 0) {
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
		  
	while(exit1 == 0 && sigquit==0 && sighup != 1){
	j=rand_r(&seed)%(dim+1);
	pthread_mutex_lock(&checklengthmutex);
	if(checkclosed[j] == 0 && checklength[j] < NUM_MAX_CUSTOMERS){
		id=j;
		cliente->cassa=id;
		//printf("cassa scelta = %d da cliente %d\n",cliente->cassa,cliente->id);
		exit1 = 1;
	}
	pthread_mutex_unlock(&checklengthmutex);	
	}	


      if(sigquit != 0 && sighup!=0) {
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
	
	clock_gettime(CLOCK_REALTIME, &start_time2);
	
	
    
	pthread_mutex_lock(&lock_cassa[id]);
	elem=cassa[id].head;
	
	if(cassa[id].head == NULL && sigquit==0){
	 cassa[id].head = head;
	 head->next = NULL;
	 }
	else if(cassa[id].head != NULL && sigquit == 0){
	while(elem->next!=NULL) elem=elem->next;
	head->next=NULL;
	elem->next=head;
	}
        min=checklength[id]++;//modifico valore della Q lenght.
        pthread_mutex_unlock(&lock_cassa[id]);

	 }
	 if(sigquit != 0) {
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
	 //printf("sono qua 1\n");
	
       /* int s=0;
        int new=dim+1;
        int r=0;
        int changed = 0;
        int tried= 0;*/
	/*while(cliente->myturn == 0 && cliente->P!=0 && sigquit == 0){
	r=rand_r(&seed)%(dim+1);
	sleep(r);
	if(tried==0){ 
       if(cliente->myturn == 1 || sigquit == 1) break;
       pthread_mutex_lock(&lock_cassa[id]); 
       pthread_mutex_lock(&checklengthmutex);
       
        if(cliente->myturn == 1 || sigquit == 1){ 
        pthread_mutex_unlock(&lock_cassa[id]); 
       pthread_mutex_unlock(&checklengthmutex);
        break; 
        }
        
       while(s<dim && sigquit != 1){
       if(checkclosed[s]==0 && checklength[id]>checklength[s] && sigquit == 0 && cliente->myturn == 0){
       (head->val)->cassa=s;
       
       checklength[id]--;
       checklength[s]++;
       pthread_mutex_unlock(&checklengthmutex);
       new=s;
       pthread_mutex_unlock(&lock_cassa[id]); 
       pthread_mutex_lock(&lock_cassa[new]);
       elem=cassa[new].head;
	
	if(cassa[new].head == NULL){
	 cassa[new].head = head;
	 head->next = NULL;
	 
	 changed=1;
	 }
	else{
	while(elem->next!=NULL) elem=elem->next;
	head->next=NULL;
	elem->next=head;
	changed=1;
        }
        pthread_mutex_unlock(&lock_cassa[new]);
               id=new;
       }
        if(cliente->myturn == 1 || sigquit == 1){ 
        pthread_mutex_unlock(&lock_cassa[id]); 
        pthread_mutex_unlock(&checklengthmutex);
        break; 
        }
        
       if(changed == 1) break;
       s++;
       }
       pthread_mutex_unlock(&lock_cassa[id]); 
       pthread_mutex_unlock(&checklengthmutex);
       tried=1;
       }  
         

	}*/
      

   pthread_mutex_lock(&customerlock[id]);
  
while(cliente->myturn == 0 && cliente->P!=0){

if(sigquit != 0) {
             pthread_mutex_unlock(&customerlock[id]);
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
pthread_cond_wait(&customercond[id],&customerlock[id]);

}
   pthread_mutex_unlock(&customerlock[id]);


clock_gettime(CLOCK_REALTIME, &current_time2);

 q_time=current_time2.tv_sec - start_time2.tv_sec;
   	
	if(sigquit != 0) {
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }



   
   
   pthread_mutex_lock(&lista_mutex);
   
   if(sigquit != 0) {
             pthread_mutex_unlock(&lista_mutex);
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }
		  
   if(sighup != 1){
   
   if(direttore == NULL && sigquit != 1){
 
        head->next = NULL;
	 direttore = head;
	 }
	else if(direttore!=NULL && sigquit!=1){
	elem=direttore;
	while(elem->next!=NULL && sigquit!=1) elem=elem->next;
	head->next=NULL;
	elem->next=head;
	}
	}
   pthread_mutex_unlock(&lista_mutex);

   while(cliente->exit != 1 && sigquit!=1 && sighup !=1){
    
    sleep(5);
   }
  
   //attendo di essere risvegliato dal thread dIRETTORE PER POTER USCIRE CON p = 0 o rand p
   

   
     if(sigquit != 0) {
	      printf("cliente spento per sigquit \n");
	      fflush(stdout);
	      activecustomers--;
	      pthread_exit (NULL); 
		  }


   activecustomers--;
   printf("ACTIVE CUSTOMERS = %d\n",activecustomers);
   fflush(stdout);

    
    clock_gettime(CLOCK_REALTIME, &current_time);

    total_time = current_time.tv_sec - start_time.tv_sec;
   

    pthread_mutex_lock(&file_lock);
    
    fprintf(statsfile,"Customer: customer_ID= %d, Number_of_products= %d, Total_time= %0.3f, Time_in_check= %0.3f , N_changes= %d \n", cliente->id , cliente->P, (double)total_time/1000, (double)q_time/1000,contatore);
    fflush(statsfile);
    pthread_mutex_unlock(&file_lock);

       free(cliente);
	printf("cliente spento in fila alla cassa %d \n",id);
	fflush(stdout);

	
    pthread_exit (NULL); 
	
}






void* director_customer(void *arg){ //create Cf->c CLIENTI, TIENE CONTO DI QUELLI ATTIVI(penso per ora P=0)
	
	int maxcustomers=0;
	
	pthread_t* customer;

	int i=0;
	maxcustomers=cf->c;
	activecustomers=maxcustomers;
	
//	int numberofcheckout=cf->k;
	
//	int Tchecks=cf->e;
	
	
	//checkout=malloc(numberofcheckout*sizeof(pthread_t));
	
	customer=malloc(maxcustomers*sizeof(pthread_t));
	
	
	for(i=0;i<(maxcustomers);i++) { //all'inizio i clienti entrano tutti C.
		if (pthread_create(&customer[i],NULL,customers,(void*)i)!=0) {
            fprintf(stderr,"fail while creating customer n %d",i);
            pthread_exit(NULL);
	}}
	
	
	while(activecustomers > 0 && sigquit!=1 && sighup!=1){

		if(sighup!=1 && sigquit!=1 && activecustomers < (cf->c-cf->e)/2){
		  printf("REALLOCO\n");
			maxcustomers=maxcustomers+(cf->e);
			int j=cf->c;
			
			if((customer=realloc(customer,maxcustomers*sizeof(pthread_t)))==NULL) {
                fprintf(stderr,"realloc fail");
            }
			
			 for(i=j;i<maxcustomers;i++){
               	if (pthread_create(&customer[i],NULL,customers,(void*) (intptr_t) i)!=0) {
                    fprintf(stderr,"fail while creating customer n %d",i);
                
                }
           
			activecustomers++;	
		}
	
	
	}
	
	sleep(2);
}


		
printf("ACTIVI %d\n",activecustomers);

for (i=0;i<maxcustomers;i++){ //attendo la fine di tutti i thread che ho creato (clienti).

        if (pthread_join(customer[i],NULL) == -1 ) {
            fprintf(stderr,"director_customer: thread join, failed!");
        }
    }
 


     exitbroadcast=1;

        done = 1;

	free(customer);
	
	printf("HO FINITO TUTTO\n");
	fflush(stdout);
	   pthread_exit (NULL); 
}



	
void* checkdirector(void *arg){ //serve un terzo thread che inizializzi i due thread
	
	int numberofcheckout=0;

	int i=0;
	int j=0;
	numberofcheckout = cf->k;
	Tchecks=cf->e;
	int out=0;
	
	check_thread=malloc(numberofcheckout*sizeof(pthread_t));
	
		for(i=0;i<(Tchecks);i++){
	pthread_mutex_lock(&checkclosedmutex); 
	checkclosed[i]=0;
	pthread_mutex_unlock(&checkclosedmutex);

	 //thread checkout venogono creati man mano.
		if (pthread_create(&check_thread[i],NULL,check,(void*) (intptr_t) i)!=0) {
            fprintf(stderr,"fail while creating checkout n %d",i);
            pthread_exit(NULL);
	}
 
	}

	

	for(i=0; i< cf->k; i++){
	if(check_thread[i]!=NULL){
        pthread_join(check_thread[i],NULL);
        checkclosed[i]=-1;
        printf("CASSA CHIUSA %d di %d\n",i,cf->e);
        fflush(stdout);
        fprintf(statsfile, "CASHIER -> | id:%d | n. products:%d | n. customers:%d | total time: %0.3f s | average service time: %0.3f s | n clousure:%d |\n",i,checkarray[i].nproducts,checkarray[i].nclients,checkarray[i].timeofservice,(float)(checkarray[i].timeofservice)/(checkarray[i].nclose),checkarray[i].nclose);

fflush(statsfile);
	}
	}
printf("CASSE CHIUSE \n");
	fflush(stdout);
	free(check_thread);
	
    pthread_exit (NULL); 
}

	
	




void* rec(void *a){
int sockt = *((int *) a);

char buffer[3000];
int count = 0;
int c=0;
int total=0;
int esci=0;
int j=0;
int val = 0;
int chiudi=0;
int apri=0;
int co=0;
listacustomer* elem;
//printf("ciao\n");

while ((count = recv(new_socket, &buffer[total], sizeof buffer - count, 0)) > 0 && sigquit!=1 && sighup != 1)
{  

   
   apri=0;
   esci = 0;
   chiudi = 0;
   int i=c;
    c=total;
    total += count;
    // At this point the buffer is valid from 0..total-1, if that's enough then process it and break, otherwise continue
  
    for( i=c;i<total;i++) {
     if(buffer[c]=='h'){ 
     printf("SIGHUP ARRIVATA\n");
     fflush(stdout);
     sighup = 1; break; }
    
    //printf("1st = %c\n",buffer[i]);//riceve (esci);
    if(buffer[i]=='c') {
	j=i-1;
	chiudi=1;
	break;
	}
	
	if(buffer[i]=='a'){
	j=i-1;
	apri=1;
	break;
	}
    if(buffer[c]=='e'){ esci = 1; break; }
    }
    if(buffer[c]=='s'){ sigquit = 1; break;}
    
    if(chiudi == 1 || apri == 1){ 
    if(c-j == 0){
	val = buffer[c] -'0';
	printf("valore=%d\n",val);
	fflush(stdout);
	}
    else { 
    val=(buffer[c] -'0')*10;
    val=(buffer[c+1] -'0')+val;
    printf("valore=%d\n",val);
    fflush(stdout);
    }
    }
    
if(esci == 1)
{

 
pthread_mutex_lock(&lista_mutex);
if(direttore!=NULL)
{ 
elem=direttore;
while(elem!=NULL)
{ 
(elem->val)->exit=1;
elem=elem->next;

}
direttore=elem;
}
pthread_mutex_unlock(&lista_mutex);  
//pthread_mutex_lock(&customercondlockdirector);              
pthread_cond_broadcast(&customerconddirector);//risveglio clienti. (li risveglio sempre appena ricevo un messaggio dal dir-
//pthread_mutex_unlock(&customercondlockdirector);
}
    
    
     if(chiudi == 1) {
    	pthread_mutex_lock(&checkclosedmutex);
    	checkclosed[val] = 1;
    	printf("closed[%d]=%d\n",val,checkclosed[val]);
    	fflush(stdout);
    	
    	pthread_mutex_unlock(&checkclosedmutex);	
	}

else if(apri == 1){
      
	pthread_mutex_lock(&checkclosedmutex);
	co=checkclosed[val];
    	checkclosed[val] = 0;
    	pthread_mutex_unlock(&checkclosedmutex);
    	if(co==-1){
    	if (pthread_create(&check_thread[i],NULL,check,(void*) (intptr_t) i)!=0) {
        fprintf(stderr,"fail while creating checkout n %d",i);
          pthread_exit(NULL);
	}
	Tchecks++;
	}
	
	}
}
//printf("count = %d\n",count);
if (count == -1)
{
printf("count = %d\n",count);

    perror("recv");
    fflush(stdout);
}
else if (count == 0)
{

  fflush(stdout);
  sighup=1;

}
while(done != 1){  
printf("WHAT im doing");   
pthread_cond_broadcast(&customerconddirector);//risveglio clienti. (li risveglio sempre appena ricevo un messaggio dal dir-            
sleep(2);
}

pthread_exit(NULL);

 } 
 




























int main(int argc, char const *argv[]) {

FILE *fileptr; // ptr to file
FILE *fileport;
 cf = configset(fileptr,argv); //setting struct config
int i=0;
int r=0;
  Createqueue();
  listacustomer* elem;
  pthread_t check1;
  unsigned int seed=time(NULL);
  r=7000+rand_r(&seed)%(8000);
  ind=0;
  pthread_t customer;
 
  pthread_t dir;
  pthread_t rc;
 
    int PORT = r;
    int server_fd, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[20] = {0}; 
    char hello[50] = "Hello from server\n"; 
    char *notifica= "no";
    int j=0;
    char* b;
    

    
      if ((fileport = fopen("superport.txt", "w")) == NULL) { 
	fprintf(stderr, "port file opening failed");
	exit(EXIT_FAILURE);
    }
    printf("R=%d\n",r);
    fflush(stdout);
    fprintf(fileport,"%d",r);
    fflush(fileport);
  

    if ((statsfile = fopen("statsfile.log", "w")) == NULL) { 
	fprintf(stderr, "Stats file opening failed");
	exit(EXIT_FAILURE);
    }
     if ((directorfile = fopen("direttore.PID", "w")) == NULL) { 
	fprintf(stderr, "Director file opening failed");
	exit(EXIT_FAILURE);
    }
 
 pid_t pID = fork();
                   if (pID == 0)  
                   {    
               
                  
          
                                    //execv("./direttore",argv);
                                    execl("/usr/bin/gnome-terminal", "gnome-terminal", "-q", "-e", "./direttore",(char*)0);
                                 
                   } 
                  
     
  /*
     printf("2=%s",notifica);  
     printf("2=%ld",strlen(notifica));  */
    // Creating socket file descriptor 
   
 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    { 
     printf("broski");
        perror("socket failed"); 
        fflush(stdout);
        exit(EXIT_FAILURE); 
    } 
     printf("creation succesuful\n");
     fflush(stdout);  
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
    printf ("bruhh");
        perror("setsockopt"); 
        fflush(stdout);
        exit(EXIT_FAILURE); 
    } 
  
    struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
    struct in_addr adr;
    adr.s_addr="127.0.0.1";
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT );  
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address)) != 0) 
    { 
        perror("bind failed"); 
    
        exit(EXIT_FAILURE); 
    }
    printf("creation succesuful\n");
  fflush(stdout);
    if (listen(server_fd, 3)) 
    { 
        printf("listen\n"); 
        
        exit(EXIT_FAILURE); 
    } 
 printf("creation succesuful\n");
     fflush(stdout);
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  ((socklen_t*)&addrlen)))<0) { 
        perror("accept"); 
        fflush(stdout);
        exit(EXIT_FAILURE); 
    } 
    printf("connection succesuful\n");
     fflush(stdout);
    /*printf("bruh=%s\n",notifica); 
    printf("%s\n",buffer); */

    int *z = malloc(sizeof(*z));          
    *z=new_socket;

  
pthread_create(&rc,NULL,(void*)rec,(void*)z);
pthread_create(&dir,NULL,(void*)dirnotify,(void*)z);
pthread_create(&check1,NULL,checkdirector,(void*) (intptr_t) j);
pthread_create(&customer,NULL,director_customer,(void*) (intptr_t) j); 



 
  
  int total=0;
  int count =0 ;

  //printf("broks");
  
  int bruh=0;
   char indice[10] ={} ;
  char val[100]={} ;
  char *usc="uscire";
  
  while(sigquit!=1 && sighup!=1){
 // printf("ciao\n");
  if(dirnot == 1){
  printf("DIRNOT = %d\n",dirnot);
  fflush(stdout);
  pthread_mutex_lock(&indice_mut);
 //  printf("%d\n",ind);
  // printf("%s\n",indice);	

  pthread_mutex_lock(&checklengthmutex);
  sprintf(indice,"%d",ind);
//  printf("%d\n",ind);
  sprintf(val,"%d",checklength[ind]);
  //printf("checklenght[%d] = %d\n",ind,checklength[ind]);
  pthread_mutex_unlock(&indice_mut);
  pthread_mutex_unlock(&checklengthmutex);
  directorsnotify[ind]=0;
  b=strcat(indice,notifica);
  b=strcat(b,val);
//  printf("%s\n",b);
  send(new_socket,b,strlen(b),0);
  //send(new_socket,notifica, strlen(notifica),0);
  bzero(b,strlen(b));
  pthread_mutex_lock(&dirnotmutex);
  dirnot=0;
  pthread_mutex_unlock(&dirnotmutex);
 }
sleep(1);
  }
  

  printf("sighup = %d\n",sighup);
  
  if(sighup==1){ 
 int true = 1;
setsockopt(new_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));

  /*
  while(uscire == 0) sleep(1);
  printf("sighup = %d\n",sighup);
  printf("sono qua?????????????????????????????? SUCIRE\n");
  send(new_socket,usc,strlen(usc),0);
  printf("ho mandato usc");*/
  }    
printf("HO FINITO MAIN\n");
fflush(stdout);
pthread_join(dir,NULL);
printf("HO FINITO dir\n");
fflush(stdout);
pthread_join(check1,NULL);
printf("HO FINITO2\n");
pthread_join(customer,NULL);
printf("HO FINITO3\n");
pthread_join(rc,NULL);
 int true = 1;
setsockopt(new_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
 fflush(stdout);
fclose(statsfile);
free(cf);
free(z);
free(cassa);
free(direttore);
free(customerlock); 
fflush(stdout);
pthread_exit(NULL);
}






config* configset(FILE* readFile,char const *argv[]){

char *fileline;
config * conf;
conf=malloc(sizeof(config));

if(strcmp(argv[1],"testfile/config.txt")==0){
    readFile = fopen("config.txt","r");
   }

else if(strcmp(argv[1],"testfile/config1.txt")==0){
    readFile = fopen("config1.txt","r");
    }

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
                        printf("FILE LINE = %s \n", fileline);
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
free(fileline);
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



void Createqueue(){
//checkclosed tiene conto delle casse che devono essere chiuse o aperte dal direttore.
//checklength dimensione fila
//directorqueue array che contiene 1 o 0, se 1 cassa invia statistiche al director di quella cassa


int i=0;

checkclosed=malloc(cf->k*sizeof(int));
int dim = cf->k;
if(!checkclosed){
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

else {
        for(i=0;i<dim;i++) checkclosed[i]=1; 
     }
     
 directorsnotify=malloc(cf->k*sizeof(int));
   if(!directorsnotify){
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

else {
        for(i=0;i<dim;i++) directorsnotify[i]=0; 
     }  
     
     
     
checklength = malloc(dim*sizeof(int));

if(!checklength){
	  
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

else {
        for(i=0;i<(dim);i++) checklength[i]=-1; 
     }




directorqueue = malloc(dim*sizeof(int));
if(!directorqueue){
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

else {
        for(i=0;i<(dim);i++) directorqueue[i]=0; 
     }
      

customercond = malloc(dim*sizeof(pthread_cond_t));
checkdirectorcond=malloc(dim*sizeof(pthread_cond_t));
checkcond=malloc(dim*sizeof(pthread_cond_t));

if(!directorqueue){
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

 
        for(i=0;i<(dim);i++){
        pthread_cond_init(&checkcond[i], NULL);
        pthread_cond_init(&customercond[i], NULL);
        pthread_cond_init(&checkdirectorcond[i], NULL);
        /*checkcond[i]=PTHREAD_COND_INITIALIZER;
	customercond[i]=PTHREAD_COND_INITIALIZER; 
	checkdirectorcond[i]=PTHREAD_COND_INITIALIZER;*/
		}
     
      
customerlock = malloc(dim*sizeof(pthread_mutex_t));

check_directorcond_lock= malloc(dim*sizeof(pthread_mutex_t));

lock_cassa = malloc(dim*sizeof(pthread_mutex_t));

check_cond_lock= malloc(dim*sizeof(pthread_mutex_t));

if(!lock_cassa || !check_directorcond_lock || !customerlock){
                fprintf(stderr, "malloc failed\n");
                exit(EXIT_FAILURE);
                }

else {
        for(i=0;i<(dim);i++){
       pthread_mutex_init(&check_cond_lock[i], NULL);
       pthread_mutex_init(&check_directorcond_lock[i], NULL);
       pthread_mutex_init(&customerlock[i], NULL);
       pthread_mutex_init(&lock_cassa[i], NULL);

		}
     }
/*if(pthread_mutex_init(&customerlockdirector, NULL) ==0) printf("HO CREATO MUTEX");
pthread_mutex_init( &checkclosedmutex, NULL);
pthread_mutex_init( &checklengthmutex, NULL);
pthread_mutex_init( &customersqmutex, NULL);
pthread_mutex_init( &directorsnotifymutex, NULL);
pthread_mutex_init( &cassamutex, NULL);
pthread_mutex_init( &customerqmutex, NULL);
pthread_mutex_init( &qopenmutex, NULL);
//pthread_mutex_init( &checkarraymutex, NULL);
pthread_mutex_init( &cusdirector, NULL);
pthread_mutex_init( &customervar, NULL);
pthread_mutex_init( &file_lock, NULL);
pthread_mutex_init( &customercondlockdirector, NULL);
pthread_mutex_init( &indice, NULL);
pthread_mutex_init( &dirnotmutex, NULL);
pthread_mutex_init( &directorupdatelock, NULL);


pthread_cond_init(&directorupdatecond ,NULL);
pthread_cond_init(&cusconddirector ,NULL);
pthread_cond_init(&customerconddirector ,NULL);*/



customerlockdirector=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
checkclosedmutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
checklengthmutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
customersqmutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
directorsnotifymutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
lista_mutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
cassamutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
customerqmutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
qopenmutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
checkarraymutex=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
cusdirector=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
indice_mut=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
customervar=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
file_lock=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
customercondlockdirector=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;




unsigned int myrand = (unsigned int)time(NULL); 
    checkarray=malloc(cf->k*sizeof(checkout));
    for(i=0;i<(cf->k);i++){
	checkarray[i].fixedtime=20+(rand_r(&myrand))%(80);
	checkarray[i].id=i;
	checkarray[i].nclients=0;
	checkarray[i].nclose=0;
	checkarray[i].nproducts=0;
	checkarray[i].timeofservice=0;
}

cassa=malloc(cf->k*sizeof(listacustomer));
direttore=malloc(sizeof(listacustomer));
direttore=NULL;
for(i=0;i<(cf->k);i++){
cassa[i].head=NULL;
}

}

















