#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define destra    1
#define sinistra  0

#define err      -1

//#define _debug

typedef struct node* pnode;


struct node{
    int j;
    int tipo;
    int value;
    pnode sx;
    pnode dx;
};

/*
** Inserimento non ricorsivo
*/
//void insert(pnode head, pnode node){
//    pnode* nhead;
//    /*
//    ** Inserimento con puntatore a puntatore
//    */
////    while ( !(*nhead) ){
////        if ( node->value < *nhead->value )
////            nhead=&(*nhead->sx);
////        else
////            nhead=&(*nhead->dx);
////        }
//
//    while ( head ) {
//        if ( node->value < head->value )
//            head=head->sx;
//        else
//            head=head->dx;
//        }
//    nhead=&head;
//    *nhead=node;
//    return;
//}

/*
** Inserimento ricorsivo
*/
pnode insert(pnode head, pnode node){
    if ( !head ) {
        //head=node;
        return node;
        }

    if ( node->value < head->value )
        head->sx=insert(head->sx, node);
    else
        head->dx=insert(head->dx, node);

    return head;
}



/*
** le successive due funzioni trasformato
** i BST in vettori, la prima in ordine
** decrescente di chiave la seconda in
** ordine crescente
*/
int vettorizzaDESC(int *v, pnode head, int i){
    if ( !head )
        return i;

    // scende a destra
//    i=vettorizzaDESC(v, head->dx, i);
    v[i]=head->j;
    i=i+1;
    i=vettorizzaDESC(v, head->sx, i);

    i=vettorizzaDESC(v, head->dx, i);

    // aggiunge al vettoreil nodo corrente

    // scende a sinistra
//    i=vettorizzaDESC(v, head->sx, i);
    return i;
}


int vettorizza(int** v, int** ut, pnode head, int ccell, int i){
    if ( !head )
        return i;

    i=vettorizza(v, ut, head->sx, ccell, i);


    v[ccell][i] =head->j;
    ut[ccell][i]=head->tipo;
    i=i+1;

    i=vettorizza(v, ut, head->dx, ccell, i);

    return i;
}



//#define nusers 3
//#define nintervalli 20
//#define ncelle 100
//#define test_file "data/Co_100_20_T_9.txt"

//#define ncelle 300
//#define test_file "data/Co_300_20_T_19.txt"

int main(int argc, char* argv[]) {
    FILE* f;


    /*
    ** Variabili comode per capire cosa
    ** sto facendo in certi punti del
    ** programma
    */
    int tasksCella;
    int cellaSorgente;
    int userTipo;
    int userDisponibili;
    int userSpostati;
    int userCost;
    int objfunc;
//    int ndestinazioni;

    int celladest;
    int cellaorig;
    int costiTot;
    int aspirantiTot;
    int nuoviAspiranti;
    int nuovoCosto;
    int mediaCosto;
    int numdest=0;

    /*
    ** matrice dei costi
    **
    ** dimensioni:
        [ncelle][ncelle][nusers][nintervalli]
    */
    int**** costs;
    /*
    ** tasks per cella
    ** dimensioni:
        [ncelle]
    */
    int* tasks;

    /*
    ** users
    ** ====================================================
    **          \ time step \ cells
    ** ----------------------------------------------------
    **          \  @1       \ $c1  $c2  $c3  ... $cn
    **  #u1     \  @2       \ $c1  $c2  $c3  ... $cn
    **          \  ...      \ $c1  $c2  $c3  ... $cn
    **          \  @i       \ $c1  $c2  $c3  ... $cn
    ** ----------------------------------------------------
    **          \  @1       \ $c1  $c2  $c3  ... $cn
    **  #u2     \  @2       \ $c1  $c2  $c3  ... $cn
    **          \  ...      \ $c1  $c2  $c3  ... $cn
    **          \  @i       \ $c1  $c2  $c3  ... $cn
    ** ----------------------------------------------------
    **          \  @1       \ $c1  $c2  $c3  ... $cn
    **  ...     \  @2       \ $c1  $c2  $c3  ... $cn
    **          \  ...      \ $c1  $c2  $c3  ... $cn
    **          \  @i       \ $c1  $c2  $c3  ... $cn
    ** ----------------------------------------------------
    **          \  @1       \ $c1  $c2  $c3  ... $cn
    **  #uN     \  @2       \ $c1  $c2  $c3  ... $cn
    **          \  ...      \ $c1  $c2  $c3  ... $cn
    **          \  @i       \ $c1  $c2  $c3  ... $cn
    ** ----------------------------------------------------
    **
    */

    /*
    ** posizione degli users nelle celle
    ** dimensioni:
        [ncelle][nusers][nintervalli]
    */
    int*** pos;

    /*
    ** 'nusers' numero di users
    ** 'nintervalli' numero di intervalli
    ** 'ncelle' numero di celle
    */
    int nusers, nintervalli, ncelle;

    /*
    ** numero di celle candidate ad essere la sorgente
    ** per una cella i-esima
    */
    int* nsorgenti;

    /*
    ** indici per i cicli
    */
    int user, time, cella, i, j, k;


    /*
    ** per evitare di allocare nuovi nodi durante l'esecuzione
    ** del programma creo un calderone di nodi dal quale pesco
    ** all'occorrenza
    **
    ** per ogni cella nel peggiore dei casi ho `ncelle` celle di
    ** provenienza; questo per gli `nusers`; facendo una stima di
    ** caso peggiore avro' bisogno di `ncelle*ncelle*nusers` nodi
    ** per i BST
    **
    ** 'newnode' punta al primo nodo del calderone disponibile
    */
    pnode* nodes;
    int newnode=0;

    /*
    ** vettore di BST (puntatori alle teste dei BST);
    ** il BST all'indice i-esimo e' della cella i-esima
    ** dimensioni:
        [ncelle][nintervalli]
    */
    pnode** bstrees;

    /*
    ** BST per ordinare le celle destinazione in ordine decrescente
    ** di costo medio.
    **
    ** Non ha bisogno di allocare nulla perche' usa gli stessi nodes
    ** degli altri BST.
    **
    ** dimensioni:
        [nintervalli]
            [ncelle]
    */
    pnode* bstDestinazioni;


    /*
    ** Vettorizzazione dei BST:
    **  - il vettore 'v' contiene gli indici delle celle di partenza
    **  - il vettore 'ut' contiene il tipo di utente per la cella i-esima
    **    del vettore 'v' corrispondente
    **
    ** dato un periodo di tempo (poi possono essere sovrascritti), nel caso
    ** peggiore avro' che per ogni cella destinazione possono essere
    ** spostati tutti i tipi di users da ogni altra cella origine
    **
    ** dimensioni:
        [ncelle][ncelle*nusers]
    */
    int** v;
    int** ut;

    /*
    ** anche le destinazioni vogliono il loro bel vettore
    ** ricavato dalla vettorizzazione del loro BST;
    **
    ** NOTA:
    **  Stavolta le celle sono ordinate in ordine decrescente
    **  di costo medio per utente.
    */
    int* vDest;

    /* passo il nome del file come parametro */
    if ( argc<2 ) {
        fprintf(stderr, "manca il file.\n");
        return -1;
        }


    clock_t end;
    clock_t start;

    f=fopen(argv[1], "r");
    assert(f!=NULL);

    fscanf(f, "%d", &ncelle);      /* leggo nr. celle      */
    fscanf(f, "%d", &nintervalli); /* leggo nr. intervalli */
    fscanf(f, "%d", &nusers);      /* leggo nr. genti      */



    /* /////////////////////////////////////////////////////////// **
    **
    ** Allocazione e preparazione della struttura dati
    **
    ** /////////////////////////////////////////////////////////// */

    bstDestinazioni=(pnode*) malloc(nintervalli*sizeof(pnode));
    assert(bstDestinazioni!=NULL);
    for (i=0; i<nintervalli; i++)
        bstDestinazioni[i]=NULL;



    /*
    ** vettore per l'ordine delle destinazioni
    **
        [ncelle]
    */
    vDest=(int*) malloc(ncelle*nintervalli*sizeof(int));
    assert(vDest!=NULL);


    /*
    ** numero di corgenti per la cella i-esima
    **
        [ncelle]
    */
    nsorgenti=(int*) malloc(ncelle*sizeof(int));
    assert(nsorgenti!=NULL);

    /*
    ** vettori BST
    **
        [ncelle]
            [nusers*ncelle]
    */
    v =(int**) calloc(ncelle, sizeof(int*));
    ut=(int**) calloc(ncelle, sizeof(int*));
    assert(  v!=NULL && ut!=NULL );
    for (i=0; i<ncelle; i++){
        v[i] =(int*) calloc(nusers*ncelle, sizeof(int));
        ut[i]=(int*) calloc(nusers*ncelle, sizeof(int));
        assert(  v[i]!=NULL );
        assert( ut[i]!=NULL );
        }


    /*
    ** calderone di nodi
    **
        [ncelle*ncelle*nusers]
    */
    k=ncelle*ncelle*nusers*nintervalli;
    nodes=(pnode*) malloc(k*sizeof(pnode));
    assert(nodes!=NULL);

    for (i=0; i<k; i++) {
        nodes[i]=(pnode) malloc(sizeof(struct node));
        assert(nodes[i]!=NULL);

        nodes[i]->j=err;
        nodes[i]->tipo=err;
        nodes[i]->value=0.0;
        nodes[i]->dx=NULL;
        nodes[i]->sx=NULL;
        }

    /*
    ** teste degli alberi per ogni cella
    **
        [ncelle]
            [nintervalli]
    */
    bstrees=(pnode**) calloc(ncelle, sizeof(pnode*));
    assert(bstrees!=NULL);
    for(i=0; i<ncelle; i++){
        bstrees[i]=(pnode*) calloc(nintervalli, sizeof(pnode));
        assert(bstrees[i]!=NULL);
        }


    /*
    ** tasks per ogni cella
    */
    tasks=(int*) malloc(ncelle*sizeof(int));


    /*
    ** multi-matrice dei costi
    **
        [ncelle]
            [ncelle]
                [nusers]
                    [nintervalli]
    */
    costs=(int****) malloc(ncelle*sizeof(int***));
    assert(costs!=NULL);
    for (i=0; i<ncelle; i++) {
        costs[i]= (int***) malloc(ncelle*sizeof(int**));
        assert(costs[i]!=NULL);
        for (j=0; j<ncelle; j++) {
            costs[i][j]= (int**) malloc(nusers*sizeof(int*));
            assert(costs[i][j]!=NULL);
            for (k=0; k<nusers; k++) {
                costs[i][j][k]=(int*) malloc(nintervalli*sizeof(int));
                assert(costs[i][j][k]!=NULL);
                }
            }
        }

    /*
    ** posizione degli users
    **
        [ncelle]
            [nusers]
                [nintervalli]
    */
    pos=(int***) malloc(ncelle*sizeof(int**));
    assert(pos!=NULL);

    for (i=0; i<ncelle; i++) {
        pos[i]=(int**) malloc(nusers*sizeof(int*));
        assert(pos[i]!=NULL);

        for (j=0; j<nusers; j++) {
            pos[i][j]=(int*) malloc(nintervalli*sizeof(int));
            assert(pos[i][j]!=NULL);
            }
        }

//  pos=(int***) malloc(nusers*sizeof(int**));
//  assert(pos!=NULL);
//  for (i=0; i<nusers;i++) {
//      pos[i]=(int**) malloc(nintervalli*sizeof(int*));
//      assert(pos[i]!=NULL);
//      for (j=0; j<nintervalli; j++) {
//          pos[i][j]=(int*) malloc(ncelle*sizeof(int));
//          assert(pos[i][j]!=NULL);
//          }
//      }

#ifdef _debug
    printf("Numero di celle: %d\n", ncelle);
    printf("Numero di users: %d\n", nusers);
    printf("Numero di intervalli: %d\n", nintervalli);
#endif



    /* /////////////////////////////////////////////////////////// **
    **
    ** Lettura dei dati dal file
    **
    ** /////////////////////////////////////////////////////////// */

    /*
    ** ignora i 'tasks per user' (1 2 3)
    */
    for (i=0; i<3; i++)
        fscanf(f, "%d", &k);


    /*
    ** carica le super-matrici dei costi
    */
    float tmp;
    for (k=0; k<nintervalli*nusers; k++) {
        fscanf(f, "%d", &user);
        fscanf(f, "%d", &time);
        for (i=0; i<ncelle; i++)
            for (j=0; j<ncelle; j++) {
                fscanf(f, "%f", &tmp);
                costs[i][j][user][time]=(int) (tmp*100);
                }
        }
#ifdef _debug
    for (user=0; user<nusers; user++)
        for (time=0; time<nintervalli; time++) {
            printf("%d %d\n", user, time);
            for (i=0; i<ncelle; i++) {
                for (j=0; j<ncelle; j++)
                    printf("%.2f ",(float) costs[i][j][user][time]/100);
                printf("\n");
                }
            }
    printf("\n");
#endif // _debug

    /*
    ** carica i tasks per le celle
    */
    for (cella=0; cella<ncelle; cella++)
        fscanf(f, "%d", &tasks[cella]);
#ifdef _debug
    for (cella=0; cella<ncelle; cella++)
        printf("%d ", tasks[cella]);
    printf("\n\n");
#endif // _debug

    /*
    ** carica le posizioni dei vari tipi di
    ** users nei diversi intervalli
    */
    for (k=0; k<nintervalli*nusers; k++) {
        fscanf(f, "%d", &user);
        fscanf(f, "%d", &time);
        for (cella=0; cella<ncelle; cella++)
            fscanf(f, "%d", &pos[cella][user][time]);
        }
    fclose(f);
#ifdef _debug
    for (user=0; user<nusers; user++)
        for (time=0; time<nintervalli; time++) {
            printf("%d %d\n", user, time);
            for (i=0; i<ncelle; i++)
                printf("%d ", pos[i][user][time]);
            printf("\n");
            }
#endif // _debug




    /* /////////////////////////////////////////////////////////// **
    **
    ** Inizio elaborazione
    **
    ** /////////////////////////////////////////////////////////// */

    start=clock();
    printf("Costruisco bstrees...\n");

    /*
    ** Costruisco i BST
    **
    ** l'idea e' sostanzialmente che per ogni cella faccio una
    ** 'classifica' delle celle di partenza dalle quali gli
    ** users si spostano piu' volentieri (a costo minore)
    **
    ** Nel BST gli users vengono mischiati e le celle di partenza
    ** potrebbero ripetersi, ma in posizioni diverse del BST. Non
    ** importa in realta' perche' tengo conto nel nodo BST sia del
    ** tipo di user che della cella di partenza.
    **
    ** NOTA:
    **  Non creo gli alberi BST per le celle che non hanno tasks.
    **  Non vengono inserite nei BST le celle che non hanno users.
    **
    **
    **
    */

    numdest=0;
    newnode=0;
    for ( time=0; time<nintervalli; time++ ) {

        /* per ogni possibile cella destinazione... */
        for ( celladest=0; celladest<ncelle; celladest++ ) {
            costiTot     =0;
            aspirantiTot =0;

            /* se non ci sono tasks ignorala..      */
            if ( tasks[celladest]==0 )
                continue;

            /* se ci sono tasks,                    **
            ** per ogni user,                       **
            **     per ogni cella,                  **
            **         inseriscila nel BST          */
            for ( cellaorig=0; cellaorig<ncelle; cellaorig++ ) {
                for ( user=0; user<nusers; user++ ) {
                    /* se non ci sono users nella cella           **
                    ** di origine ignorala..                      */
                    if ( pos[cellaorig][user][time]==0 )
                        continue;

                    nuoviAspiranti=pos[cellaorig][user][time];
                    nuovoCosto    =costs[celladest][cellaorig][user][time];

                    costiTot     += nuovoCosto*nuoviAspiranti;
                    aspirantiTot += nuoviAspiranti;

                    /* imposto il nodo */
                    nodes[newnode]->j=cellaorig;
                    nodes[newnode]->tipo=user;
//                    nodes[newnode]->value=costs[cellaorig][celladest][user][time];
                    nodes[newnode]->value=costs[cellaorig][celladest][user][time]/(user+1);

                    /* inserisco il nodo nel BST della cella di    **
                    ** destinazione                                */
                    bstrees[celladest][time]=insert(bstrees[celladest][time], nodes[newnode]);

                    /* uso il nodo disponibile successivo          */
                    newnode++;
                    }
                }

            mediaCosto=(int) (costiTot / aspirantiTot);
            nodes[newnode]->j    = celladest;
            nodes[newnode]->tipo = aspirantiTot;
            nodes[newnode]->value= mediaCosto;

            bstDestinazioni[time]=insert(bstDestinazioni[time], nodes[newnode]);
            numdest++;
            newnode++;
            }
        }
        end=clock();
        printf("Used nodes %d\nNum of destinations %d\n", newnode, numdest);
        printf("BST insertions time is: %f.\n",
              ((double)end-start)/CLOCKS_PER_SEC);





//    int minobjfunc=99999999;
//    int offsetj;
//    int innObjFunc;
//    int minInnObjFunc;
    int spostati[ncelle][nusers];


    for (cella=0; cella<ncelle; cella++)
        for (user=0; user<nusers; user++)
            spostati[cella][user]=0;

    objfunc=0;
//    offsetj=0;


    for ( time=0; time<nintervalli; time++ ) {
//        minInnObjFunc=99999999;
//        #ifdef _debug
//        printf("Intervallo %d", time);
//        #endif // _debug

//        ndestinazioni=vettorizzaDESC(vDest, bstDestinazioni[time], 0);
//
//        /* aggiunta successiva: provo a partire da celle diverse tra quelle
//        ** presenti tra le celle destinazione
//        ** per ogni ordine mi calcolo il parziale dell'obj. func.
//        **
//        ** il minimo parziale lo agguingo all'obj. func. generale e passo
//        ** all'intervallo di tempo successivo */
//        for (offsetj=0; offsetj<ndestinazioni; offsetj++) {
//            #ifdef _debug
//            printf("\toffset %d\n", offsetj);
//            #endif // _debug
//
//            innObjFunc=0;

            for ( j=0; j<ncelle; j++ ) {        /* invece di ciclare indistintamente sulle celle    **
                                                ** prendo le celle destinazione in ordine, piu'     **
                                                ** precisamente in ordine inverso di costo medio    **
                                                ** per utente che vi si reca                        */
//                cella=vDest[(j+offsetj)%ndestinazioni];
//                cella=vDest[j];
                nsorgenti[cella]=vettorizza(v, ut, bstrees[cella][time], cella, 0);


                tasksCella=tasks[cella]; /* task da eseguire nella cella destinazione */
                for ( i=0; i<nsorgenti[cella] && tasksCella>0; i++ ) {
                    cellaSorgente=v[cella][i];  /* indice cella sorgente */
                    userTipo=ut[cella][i];  /* tipo di user da spostare  */

//                    alcune righe sono commentate perche' prima trovato una sola soluzione e
//                    modificavo le strutture dati originali direttamente
//                    userDisponibili=pos[cellaSorgente][userTipo][time]; /* user disponibili nella cella sorgente */
                    userDisponibili=pos[cellaSorgente][userTipo][time] - spostati[cellaSorgente][userTipo]; /* user disponibili nella cella sorgente */
                    userSpostati=0; /* users spostati dalla nuova cella sorgente */
                    userCost=costs[i][cella][userTipo][time]; /* costo per spostare un user di tipo `userTipo`            **
                                                              ** dalla cella sorgente `i` alla cella destinazione `cella` */



                    while( tasksCella>0 && userDisponibili>0 && (tasksCella-(userTipo+1))>=0 ) {
                        userDisponibili--; /* tolgo un user dalla sorgente...                        */
                        userSpostati++;    /* ...e lo mando a lavorare in destinazione.              */
                        tasksCella -= (userTipo+1); /* aggiorno il numero di task rimasti da svolgere */
//                        objfunc += userCost; /* incremento consequentemente la object function.      */
                        objfunc += userCost; /* incremento consequentemente la object function.      */
                        }

                    spostati[cellaSorgente][userTipo] += userSpostati;
//                    pos[cellaSorgente][userTipo][time] -= userSpostati; /* gli user che ho spostato per la cella destinazione **
//                                                                        ** non sono piu' disponibili per le altre celle       */
                    }

                }

//            if (innObjFunc < minInnObjFunc) {
//                minInnObjFunc = innObjFunc;
//                #ifdef _debug
//                printf("\t\tAl tempo %d, c'e' un parziale di obj. func. migliore: %d\n", time, innObjFunc);
//                #endif // _debug
//                }
//
            for (cella=0; cella<ncelle; cella++)
                for (user=0; user<nusers; user++)
                    spostati[cella][user]=0;
//            }

//        objfunc += minInnObjFunc;
//        #ifdef _debug
//        printf("\tAggiornata l'obj. func. (+ %d) ora e' %d\n", minInnObjFunc, objfunc);
//        #endif // _debug
    }

    fprintf(stdout, "\n\nOBJECT FUNCTION: %d\n\n", objfunc);





    end=clock();
    printf("Hello motherfucker! Time is: %f.\n",
          ((double)end-start)/CLOCKS_PER_SEC);


//    free(tasks);
//    for (i=0; i<nintervalli; i++)

    return 0;
}
