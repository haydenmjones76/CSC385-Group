#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define BUFFERSIZE 3
sem_t taSem;
sem_t studentSem;
sem_t sleepSem;
int buffer[BUFFERSIZE];
pthread_mutex_t mutex;
int out = 0;
int in = 0;
int nStudents = 15;
bool isSleep = false;
///single TA process
void *ta_actions(void *number){
	
	int help_time; 						//variable duration students will be helped
    int count = *((int *)number); 		//total students left to help
		
    while(count > 0){					//loop until no students remain
		//if someone is waiting in the seats
		if(buffer[in] != 0){
			//acquire locks
			sem_wait(&studentSem);	
			pthread_mutex_lock(&mutex);
			help_time = rand() % 5;
			
			//Help student, increment buffer, post to console, decrementloop qualifier
			printf("Helping student %d for %d seconds.\n", buffer[in], help_time);
			buffer[in] = 0;
			in = (in+1) % BUFFERSIZE;
			count--;
			
			//release locks
			pthread_mutex_unlock(&mutex);
			sem_post(&taSem);

			//to simulate helping the student in office
			sleep(help_time); 
		}
		//if no one is waiting in the seats.
		else{
			printf("No students waiting. Sleeping.\n");
			//"sleeping". is waiting here until time to be "awoken" by semaphore post
			isSleep = true;
			sem_wait(&sleepSem);
		}
	}
}

void *student_actions(void *student_id){
    int time = rand() % 5;
    int id_student = *((int *)student_id);
	
	while(1){ //while 1 not while(true) how odd
		//inverse logic of TA, if a seat is open, continue in
		if(buffer[out] == 0){
			//acquire locks
			sem_wait(&taSem);
			pthread_mutex_lock(&mutex);
	
			//add self to a seat, increment buffer, post to console
			buffer[out] = id_student;
			printf("\t\tStudent %d takes seat %d\n", id_student, out);
			out = (out+1) % BUFFERSIZE;
			
			//check if every other seat is empty, if so "wakes" the TA with a semaphore
			if((buffer[out] % BUFFERSIZE) == 0 & (buffer[out+1] % BUFFERSIZE) == 0 & isSleep == true){
				sem_post(&sleepSem);
			}
			
			//release locks
			pthread_mutex_unlock(&mutex);
			sem_post(&studentSem);
			
			break;
		}
		//if all seats are full, returns to programming
		else{	
			printf("\tStudent %d is programming for %d seconds.\n", id_student, time);
			sleep((rand() % 10)); //goes away and comes back later to check
		}
	}
}

int main(int argc, char** argv) {
        //checks if custom number of students has been passed to use, else use default 15
		if(argc > 1){
			nStudents = atoi(argv[1]);
			if(nStudents == 0){ //ensures an actual int was passed, otherwise kicks them out.
				printf("Invalid parameter. Int values to correspond to number of students only.\n");
				exit(0);
			}
		}
		printf("%d students need help.\n",nStudents);
		
		//assign values and initialize semaphores/mutex
        pthread_t stu[nStudents], ta;
        pthread_mutex_init(&mutex, NULL);
        sem_init(&taSem, 0, BUFFERSIZE);
        sem_init(&studentSem, 0, 0);
		sem_init(&sleepSem, 0, 0);
        int student_ID[nStudents]; //simple naming scheme for students.
		srand(1);
		
        //assign default values for buffer to 0
        for(int i = 0; i < BUFFERSIZE; i++){
                buffer[i] = 0;
        }

        //create pthreads
        pthread_create(&ta, NULL, (void *)ta_actions, &nStudents); //passes total number of students to TA
        for(int i = 0; i < nStudents ; i++){
                student_ID[i] = i+1;
                pthread_create(&stu[i], NULL, (void *)student_actions, &student_ID[i]); //passes unique name along to each student
        }
		
        //pthread teardown
        pthread_join(ta, NULL);
        for(int i = 0; i < nStudents; i++){
                pthread_join(stu[i], NULL);
        }

        //cleanup
        pthread_mutex_destroy(&mutex);
        sem_destroy(&taSem);
        sem_destroy(&studentSem);

        return 0;
}