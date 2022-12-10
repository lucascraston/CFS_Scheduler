#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>


#define NUM_THREADS 4
#define NUM_PROCESSES 3
//min and max macros for future calcs
#define MAX(X, Y) (X > Y ? X:Y)
#define MIN(X, Y) (X > Y ? Y:X)


//here we define our scheduling types used
typedef enum scheduling_policy{
    RR = 0,
    FIFO =1,
    NORM= 2
}scheduling_policy;
//this struct holds our process info
typedef struct process{
    //int priority;///normal,RR, FIFO
    int PID;
    int SP; 
    int DP;
    long int time_slice, acu_time_slice, remain_time;
    pthread_t thread_id;
    scheduling_policy sched;
    int active;
}process;
//this is the main struct that will hold an array of our processes
typedef struct queues{
    process arr[5];
    int size;
}queues;
//create our main Q
queues qued[NUM_THREADS][3];

//helper variables 
int threads_finished = 0;
int producer_finished =0;


//our producer is responsible for reading an input file, parsing the data,
//and adding it our Q
void *producer(void*arg){
    //variables to help array traversal
    int count = 0;
    int RR = 0;
    int FIFO = 0;
    int NORM = 0;

    //opeb our file for reading
    FILE *infile = fopen("input.txt","r");
    if ( infile == NULL ) {  // error checking with fopen call
        printf("Unable to open file."); 
        exit(1);
    }
    //these will hold the data that we read
    int sp,ts;
    char data[7];
    //we read 20 'processes'
    //loop through our 2D array
    for(int i =0;i<NUM_THREADS;i++){
        for(int j = 0; j<5;j++){
            int dat = fscanf(infile,"%s%d%d",&data,&sp,&ts);    //read data from file
            printf("%s: %d: %d:\n",data,sp,ts);
            //check the SP, then we know where it goes
            if(sp<100){
                qued[i][0].arr[j].PID = count;
                qued[i][0].arr[j].SP = sp;
                qued[i][0].arr[j].DP = sp;
                qued[i][0].arr[j].remain_time = ts; 
                qued[i][0].arr[j].time_slice =100;
                qued[i][0].arr[j].acu_time_slice =0;
                //check the scheduling type
                if(strcmp(data,"FIFO")==0){
                    qued[i][0].arr[j].sched =1;
                }else{
                    qued[i][0].arr[j].sched =0;
                }
                
                qued[i][0].arr[j].active =1;
                qued[i][0].size +=1; 

            }else if(sp>=100 && sp<=129){
                //anything above 100, we know is RQ1&2
                qued[i][1].arr[j].PID = count;
                qued[i][1].arr[j].SP = sp;
                qued[i][1].arr[j].DP = sp;
                qued[i][1].arr[j].remain_time = ts;
                qued[i][1].arr[j].time_slice =100;
                qued[i][1].arr[j].acu_time_slice =0;
                qued[i][1].arr[j].sched =2;
                qued[i][1].arr[j].active =1;
                qued[i][1].size +=1; 
            //if in RQ1 or 2, sched type is NORM
            }else if(sp>=130 && sp<140){
                qued[i][2].arr[j].PID = count;
                qued[i][2].arr[j].SP = sp;
                qued[i][2].arr[j].DP = sp;
                qued[i][2].arr[j].remain_time = ts;
                qued[i][2].arr[j].time_slice =100;
                qued[i][2].arr[j].acu_time_slice =0;
                qued[i][2].arr[j].sched =2;
                qued[i][2].arr[j].active =1;
                qued[i][2].size +=1; 
   
            } 
            //count is used as our PID, increment here
         count++;      
        }
        //this is our insert sort for RQ0
        for(int k= 1;k<5;k++){
            process key = qued[i][0].arr[k];
            int w = k-1;
            while(w>=0 && qued[i][0].arr[w].SP> key.SP){
                qued[i][0].arr[w+1] = qued[i][0].arr[w];
                w = w-1;
            }
            qued[i][0].arr[w+1] = key;
        }
        //this is our insert sort for RQ1s
        for(int k= 1;k<5;k++){
            process key = qued[i][1].arr[k];
            int w = k-1;
            while(w>=0 && qued[i][1].arr[w].SP> key.SP){
                qued[i][1].arr[w+1] = qued[i][1].arr[w];
                w = w-1;
            }
            qued[i][1].arr[w+1] = key;
        }
        //this is our insert sort for RQ2
        for(int k= 1;k<5;k++){
            process key = qued[i][2].arr[k];
            int w = k-1;
            while(w>=0 && qued[i][2].arr[w].SP> key.SP){
                qued[i][2].arr[w+1] = qued[i][2].arr[w];
                w = w-1;
            }
            qued[i][2].arr[w+1] = key;
        }

    }    
    //update console
    printf("Producer finished\n");
     
    fclose(infile);
    //this var lets us know to start our threads
    producer_finished =1;
}



//consumer is resonsibe for 'executing' processes
void *consumer(void*arg){   
    //create new Q from the one passed in
    queues *processes = (queues *)arg;
    process right_now;
    //more helper variables
    int counter=0;
    //'execute' our processes until they are all done
    do{
        for(int i =0; i< 3;i++){    //loops through RQs
            for(int j =0;j<5;j++){  //loops through RQX.arr[]
                right_now =processes[i].arr[j];
                //verify our process has not already been finished
                if(right_now.active==1){
                    //print to console, we have successfulyl grabbed a process
                    printf("Pulled from Queue:%d\n",i);
                    printf("process info:\nprocess PID: %d\nStatic Priority: %d\nDynamic Priority: %d\nRemaining execution time: %dms\nTime slice: %dus\nAccumulated time slice: %dms\nThread ID: %d\nscheduling type: %d\n\n",
                        right_now.PID, right_now.SP, right_now.DP, right_now.remain_time, right_now.time_slice, right_now.acu_time_slice, right_now.thread_id, right_now.sched);

                    //switch case for each scheduling type
                    switch(right_now.sched){
                        //Case RR, executes for time slice, fixed SP, then updates
                        case(RR):
                            //calculate the time slice we will allocate        
                            right_now.time_slice = right_now.SP < 120 ? ((140 - right_now.DP)*20000):((140 - right_now.DP)*5000);
                            usleep(right_now.time_slice);
                            right_now.acu_time_slice += right_now.time_slice / 1000;
                            right_now.remain_time -= right_now.time_slice / 1000;
                            printf("RR process info after execution:\nprocess PID: %d\nStatic Priority: %d\nDynamic Priority: %d\nRemaining execution time: %dms\nTime slice: %dms\nAccumulated time slice: %dms\nThread ID: %d\n\n",
                            right_now.PID, right_now.SP, right_now.DP, right_now.remain_time, right_now.time_slice/1000, right_now.acu_time_slice, right_now.thread_id);
                            if(right_now.remain_time <=0){
                                right_now.active=0;
                                counter+=1;
                            }
                            break;
                        //fifo executes for entire time until done
                        case(FIFO):
                            usleep(right_now.remain_time);
                            right_now.acu_time_slice = right_now.remain_time;
                            right_now.remain_time =0;
                            printf("FIFO process info after execution:\nprocess PID: %d\nStatic Priority: %d\nDynamic Priority: %d\nRemaining execution time: %dms\nTime slice: %dms\nAccumulated time slice: %dms\nThread ID: %d\n\n",
                            right_now.PID, right_now.SP, right_now.DP, right_now.remain_time, right_now.time_slice/1000, right_now.acu_time_slice, right_now.thread_id);
                            right_now.active  =0;
                            
                            counter+=1;
                            break;
                        //norm is low priority, executes for time slice then updates
                        case(NORM):
                            int bonus = rand() % 10;
                            right_now.time_slice = right_now.SP < 120 ? ((140 - right_now.DP)*20000):((140 - right_now.DP)*5000);
                            right_now.DP = MAX(100, MIN(right_now.SP - bonus + 5, 139));
                            usleep(right_now.time_slice);
                            right_now.acu_time_slice += right_now.time_slice / 1000;
                            right_now.remain_time -= right_now.time_slice / 1000;
                            printf("NORM process info after execution:\nprocess PID: %d\nStatic Priority: %d\nDynamic Priority: %d\nRemaining execution time: %dms\nTime slice: %dms\nAccumulated time slice: %dms\nThread ID: %d\n\n",
                            right_now.PID, right_now.SP, right_now.DP, right_now.remain_time, right_now.time_slice/1000, right_now.acu_time_slice, right_now.thread_id);
                            if(right_now.remain_time <=0){
                                right_now.active=0;
                                counter+=1;
                                break;
                            }          
                            //since NORM uses DP, we need to adjust it in the Q
                            //this puts it into RQ1 if <130
                            if(right_now.DP>=100 && right_now.DP<=129){
                                for(int m =0;m<5;m++){
                                    if(processes[1].arr[m].active ==0){
                                        processes[1].arr[m] = right_now;
                                        right_now.active =0;
                                        break;
                                    }

                                }
                            } 
                            //this puts it in RQ2 if >= 130
                            else if(right_now.DP>=130 && right_now.DP<=139){
                                for(int m =0;m<5;m++){
                                    if(processes[2].arr[m].active ==0){
                                        processes[2].arr[m] = right_now;
                                        right_now.active =0;
                                        break;
                                    }

                                }
                            } 
                            //now we need to re-prioritize
                            //this is our insert sort for RQ1s
                            for(int k= 1;k<5;k++){
                                process key = processes[1].arr[k];
                                int w = k-1;
                                while(w>=0 && processes[1].arr[w].SP> key.SP){
                                    processes[1].arr[w+1] = processes[1].arr[w];
                                    w = w-1;
                                }
                                processes[1].arr[w+1] = key;
                            }
                            //this is our insert sort for RQ2
                            for(int k= 1;k<5;k++){
                                process key = processes[2].arr[k];
                                int w = k-1;
                                while(w>=0 && processes[2].arr[w].SP> key.SP){
                                    processes[2].arr[w+1] = processes[2].arr[w];
                                    w = w-1;
                                }
                                processes[2].arr[w+1] = key;
                            }
                            
                    }  
                   processes[i].arr[j]= right_now;     //update our Q 
                }
            }

        }
    }while(counter<5);  //make sure all processes are done, then update our main
    threads_finished++;
    printf("thread finished\n");
}

//main function creates all threads and manages the Q distribution
int main(void){

    //create required vars for making threads
    int res;
    pthread_t c_thread[NUM_THREADS];
    pthread_t p_thread;
    pthread_attr_t attr;

    void *thread_result;
    struct sched_param sched;
    //create thread attribute settings
    res = pthread_attr_init(&attr);
    if(res != 0){
        printf("failed to create attribute\n");
    }
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(res != 0){
        printf("failed to set detached\n");
    }
    res = pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    if(res != 0){
        printf("failed to make sched policy\n");
    }
    int max_priority = sched_get_priority_max(SCHED_OTHER);
    int min_priority = sched_get_priority_min(SCHED_OTHER);
    sched.sched_priority =min_priority;
    //initial set of detach param
    res = pthread_attr_setschedparam(&attr, &sched);
    if(res != 0){
        printf("failed to make scheduling params\n");
    }
    //after attributes are done,  create producer thread
    res = pthread_create(&(p_thread), &attr, producer, NULL);
        //error checking for thread
    if (res != 0) {
        perror("producer thread creation failed");
        exit(EXIT_FAILURE);
    }   
    //set detached paramter again
    sched.sched_priority = max_priority;
    res = pthread_attr_setschedparam(&attr, &sched);
    if(res != 0){
        printf("failed to make scheduling params\n");
    }
    //wait until producer has finished producing processes before making threads
    while(!producer_finished){
        usleep(5000);
    }

    //create the trheads
    for(int i = 0; i < NUM_THREADS; i++) {
        res = pthread_create(&(c_thread[i]), &attr, consumer, (void *)qued[i]);
        //error checking for thread
        if (res != 0) {
            perror("consumer thread creation failed");
            exit(EXIT_FAILURE);
        }else{
            for(int j = 0; j<NUM_PROCESSES;j++){
                for(int z = 0; z<5;z++){
                    qued[i][j].arr[z].thread_id =c_thread[i];   //this tells us which thread handles which 'process'
                }   
            }
        }
    }   
    //wait until all threads has reported to be complete
    while(threads_finished < NUM_THREADS){}; 
    exit(EXIT_SUCCESS); //done!
}






