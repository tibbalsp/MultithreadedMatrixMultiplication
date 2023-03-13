/*
 *  Author: Patrick Tibbals
 *
 *  prodcons module
 *  Producer Consumer module
 *	
 * 	Manages the buffer to generate and multiple the desired quantity of operations
 * 
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"


// Define Locks, Condition variables, and so on here
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t full = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t  putLock = PTHREAD_MUTEX_INITIALIZER;

//Counter 
counter_t * prod;
counter_t * cons;

int fill = 0;
int count = 0;
int use = 0;




int put(Matrix *value) {
	bigmatrix[fill] = value;
	fill = (fill+1) % BOUNDED_BUFFER_SIZE;
	count++;
	return 0;
}

Matrix * get() {
	if(bigmatrix != NULL){
		Matrix *tmp = bigmatrix[use];
		use = (use+1) % BOUNDED_BUFFER_SIZE;
		count--;
		return tmp;
	}
	return NULL;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg) {

	// Stats for producers
	ProdConsStats * pcs = malloc(sizeof(ProdConsStats));
	init_ProdConsStats(pcs);
	Matrix * m;
	
	// Stats for producers
  	while(get_cnt(arg) < NUMBER_OF_MATRICES){

		pthread_mutex_lock(&putLock);
		while (count == BOUNDED_BUFFER_SIZE) {
			if(get_cnt(arg) >= NUMBER_OF_MATRICES){
				pthread_cond_signal(&full); 
				pthread_mutex_unlock(&putLock);
				return (void *) pcs;
			}

		    	pthread_cond_wait(&empty, &putLock);
        }
        
	if(get_cnt(arg) < NUMBER_OF_MATRICES){
	       	m = GenMatrixRandom();
	       	put(m);
	       	
	        increment_cnt(arg);
	        int val = pcs->matrixtotal;
		val++;
		pcs->matrixtotal = val;
		
		val = pcs->sumtotal;
		val = val + SumMatrix(m);
		pcs->sumtotal = val;

		m = NULL;
		pthread_cond_signal(&full); 
	}

        pthread_mutex_unlock(&putLock);

    }

	return (void *) pcs;
}



// Matrix CONSUMER worker thread
void *cons_worker(void *arg) {
	
	ProdConsStats * pcs = malloc(sizeof(ProdConsStats));
	init_ProdConsStats(pcs);

	// Cosume number of matrices
	while(get_cnt(arg) < NUMBER_OF_MATRICES) {
		pthread_mutex_lock(&putLock);
		// Wait if buffer is empty
		while(count == 0){
			// If comsumed all then return
			if(get_cnt(arg) >= NUMBER_OF_MATRICES){
				pthread_cond_signal(&empty);
				pthread_mutex_unlock(&putLock);
				return (void *) pcs;
			}
			pthread_cond_wait(&full, &putLock);
		}
		
        	Matrix *m1 = get();
        	
       	        // Increment counts
       	        increment_cnt(arg);		
		int val = pcs->matrixtotal;
		val++;
		pcs->matrixtotal = val;
		
		val = pcs->sumtotal;
		val = val + SumMatrix(m1);
		pcs->sumtotal = val;
		
		
		Matrix *m2 = NULL;
		Matrix *m3 = NULL;

		while( m3 == NULL){
			if(m2!=NULL){
	    			FreeMatrix(m2);
				m2 = NULL;
			}

			while(count == 0){
				//if comsumed all then return
				if(get_cnt(arg) >= NUMBER_OF_MATRICES){
					pthread_cond_signal(&empty);
					pthread_mutex_unlock(&putLock);

					return (void *) pcs;
				}

				pthread_cond_signal(&empty);
				pthread_cond_wait(&full, &putLock);
			}
		
			m2 = get();
			
	       	        //Increment counts
	       	        increment_cnt(arg);		
			val = pcs->matrixtotal;
			val++;
			pcs->matrixtotal = val;
			
			val = pcs->sumtotal;
			val = val + SumMatrix(m2);
			pcs->sumtotal = val;
			
			m3 = MatrixMultiply(m1,m2);
		
		
		}


		DisplayMatrix(m1,stdout);
		printf("    X\n");
		DisplayMatrix(m2,stdout);
		printf("    =\n");
		DisplayMatrix(m3,stdout);
		printf("\n");
		
		val = pcs->multtotal;
		val++;
		pcs->multtotal = val;
	
	    	FreeMatrix(m3);
	    	FreeMatrix(m2);
	    	FreeMatrix(m1);

	   	m1=NULL;
	    	m2=NULL;
	    	m3=NULL;	
		
		pthread_cond_signal(&empty);
    		pthread_mutex_unlock(&putLock);	

	}
	
	return (void *) pcs;

}

void init_ProdConsStats(ProdConsStats* structs){
	structs->multtotal = 0;
	structs->matrixtotal = 0;
	structs->sumtotal = 0;

}

