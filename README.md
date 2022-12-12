## CFS Scheduling algorithm
This program implements a simplified linux Comlpetely Fair Scheduler(CFS) using threads. The Process Control Block (PCB) is held in a user defined struct and holds key details that define the process. A producer thread is made and reads in processses from a text file. The text file contains the processes scheduling policy, the priority, and the amount of time it needs to run. The producer then sorts these processes, based on their priority, into ready queues. The lower the the priority number, the higher priority the task is. 

For this simplified program, there are 3 Queues, Queue 0 is for [0:99) priority, Q2 -> [100:129), Q3 -> [130:139).
We then generate 4 consumer threads, each thread is passed its own set of priority queues. The consumers job is to go through the queues and run the highest priority task.

When a thread picks up a process, the process can have 3 execution styles, either FIFO, Round Robin (RR), or Normal (NORM). Based off the PCB of the process, the consumer knows how to run the process, and it does. After the 'execution' of the process, the consumer then updates the processes info, and then either places it back into the Queue to be run again, or removes it if it has completed. This continues until all processes have completed their execution time.

In order to decide the run time of each process, we calculate its quantum time slice based off its static priority. FIFO processes, always run for their entire time slice, i.e. they run to completion once they have been picked up from a thread.

RR processes, run for their calculated time slice, they only have a static priorit which is uded to calculae this.
Normal processes have a static priority, and a dynamic priority which is used after to alter their priority and redistribute run time.

All in all, this is a pretty cool project. I want to add some things here: The code does not focus on efficiency, the primary focus is to understand and implement a super basic scheduling algorithm. I used an array to store the ready queues, and used inefficent sorting algorithms like insertion sort to filter the priorities. In a real Operating System, different data structures would be used.