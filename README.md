# system

system is a set of common patterns and usefull libs

## Table of Contents
 * [Thread](#Thread)


## Thread

Thread is a part of system library. Thread class provide next functionality -
1. Different run idioms - 
1.a. Standart while loop idiom - call function while thread in running state
1.b. With service function - sama as Standart while loop idiom but with for proceeding additional logic.
1.c. With pre and post main function - sama as Standart while loop idiom but with pre and post while loop function (for initialization thread specific structures).
1.d. With service and pre and post functions - 2b + 2c
2. Monitoring - can check how many times we proceed functions and how long it was.
3. OS specific functions - set name thread and set affinity thread
