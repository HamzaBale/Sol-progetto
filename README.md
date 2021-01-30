Implementation of a supermarket system for the course "operative system and laboratory" at IT university.

How to use the program:

Make -> To build the .c files

Make test1 -> To run the test1

Make test2 -> To run the test2

Make clean -> To clean the directory

To modify config parameters -> Modify the ./testfile/config.txt

config parameters:

K -> Number of supermarket checkouts

C -> Number of max customers in the supermarket initially

E -> Number of customers that need to exit before make enter other E customers

T -> Min time for customers to buy

P -> Number of max products for each customers

S -> Seconds to do a product by the cashiers

S1 -> If the are S1 queues with <=1 customers -> Close one of them

S2 -> If the is a queue with more then S2 customers in queue -> Open another supermarket checkouts

smopen -> How much sm are opened at the start of the program

directornews -> Update time used from sm to send queue length to director
