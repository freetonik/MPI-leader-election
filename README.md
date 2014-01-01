# Simulation of “All-The-Way”, “As-Far-As-Can” and “Stages” leader election protocols in ring using Message Parsing Interface and C++

## Idea

Message Parsing Interface (MPI) allows to create applications for a particular type of distributed environment: a cluster (computers in a network). The idea of this project is to emulate three basic leader election protocols for rings (“All-The-Way”, “As-Far-As-Can” and “Stages”) using MPI and analyze real-world results regarding time and message complexity.

## Cluster

Emulations were executed on Carleton University School of Computer Science lambda-network, consisting of 16 computers, 12 of which have Intel Core 2 Duo processor and 4 have Intel Quad Core processor.  A ring topology was emulated by using the following structure to send or receive messages over the network:

`sending to the right: destination = (rank+1)%size`     
`sending to the left: destination = (rank-1)%size`
        
where rank is the number of a processor (from set {0,1,...,15}), and size is the amount of computers in network (in our case at most 16). 
Number of processor is equal to its ID, so the structure of the network is as follows:

![]([https://raw.github.com/freetonik/MPI-leader-election/master/images/1.png]

This configuration is not only most straightforward to imagine, but also happens to be the worst case configuration for protocol “As-Far-As- Can”.

To compile a MPI program:

`mpic++ ./alltheway.cpp -o alltheway`

To run:

`mpirun -np N --hostfile hosts ./alltheway [argument]`

where N is the number of computers to use.

## PROTOCOL “ALL THE WAY”

This is the simplest protocol for electing a leader in a ring. Each node sends its ID to the right. Each message travels around the ring and comes back to its originator. Every message has a counter which is incremented every time any node forwards it. Every node receives IDʼs of every other node, so every node can keep track of minimum value. Each node also has its own local counter, counting the number of messages seen so far. When a node receives its own ID and its local timer is equal to messageʼs counter, a current minimum is declared a leader.

Initially, each node has two arrays: inmsg[2] for holding received message consisting of two values - ID and counter, and msg[2] for holding initial values to send. Each node also knows N (the number of computers in the network), but since this notion is not in the standard set of assumptions, the variable ʻsizeʼ is not used anywhere except for sending and receiving (as was explained earlier). Each node also has local counter ʻlcounterʼ and local message counter to keep track of the total number of messages sent and received.

First step of the program is for every node to send its ID and counter=0 to the right.

`MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);`

Then, a while loop executes until ʻterminatedʼ boolean variable is set to true. Within a loop, each node receives a message from the left – either from the first send or from forwarding a message. A local minimum of each nodes is updated if needed. At this point the program checks if received message has the same ID as the node and nodeʼs local counter is equal to messageʼs counter. If so, the current minimum node is declared a leader. Otherwise, received message is being forwarded to the right with counter incremented by one.

This implementation also has ability to send X dummy messages between nodes 0 and 1 after execution of protocol is finished. This is done for analysis purposes, and motivation will be explained clearer in the next section. To add dummy messages, run the program with an argument:

`mpirun -np N --hostfile hosts ./alltheway.cpp X`

where N is the number of computers, and X is the number of
additional messages.

The program keeps track of the time spent by execution. The time includes the time spent by communicating additional messages, if any. The program also counts the total number of messages by collecting the corresponding counters in the end and summing them. The final output for default configuration of 16 processors and 32 additional messages looks like this:

lambda01:> mpirun -np 16 --hostfile hosts ./alltheway 32
Node 0 declares node 0 a leader!
Node 1 declares node 0 a leader!
Node 15 declares node 0 a leader!
Node 3 declares node 0 a leader!
Node 2 declares node 0 a leader!
Node 14 declares node 0 a leader!
Node 5 declares node 0 a leader!
Node 7 declares node 0 a leader!
Node 9 declares node 0 a leader!
Node 11 declares node 0 a leader!
Node 13 declares node 0 a leader!
Node 6 declares node 0 a leader!
Node 10 declares node 0 a leader!
Node 12 declares node 0 a leader!
Node 4 declares node 0 a leader!
Node 8 declares node 0 a leader!
Elapsed time: 0.0201511 seconds
Total messages sent and received: 136
Additional messages sent: 32

## PROTOCOL “AS FAR AS CAN”

This protocol is an obvious modification of previous protocol. Instead of forwarding all every message, a node filters them: forwards only those messages whose ID is smaller than nodeʼs current local minimum.

The way MPI handles message passing is the following: a node that needs to send a message originates “MPI_SEND” command; a node that needs to receive a message originates “MPI_RECV” command. Consider the following case:

![]([https://raw.github.com/freetonik/MPI-leader-election/master/images/2.png]

Each nodes executes “MPI_SEND” function to send its ID to the right. At the next step, each node executes “MPI_RECV” to receive a message from the left. Then, all the nodes forward the message further, except for node 0 (its ID < received ID 15).

![]([https://raw.github.com/freetonik/MPI-leader-election/master/images/3.png]

In this situation, every node executes “MPI_RECV” to receive the forwarded message, but since node 0 didnʼt forward anything, node 1 will wait forever. This situation is due to the way MPI passes messages and can be avoided by either sending additional notification message prior to any communication to specify the type of future communication or its absence, or by building up additional logic to the protocol. Either way will affect both time and message complexity, so it was decided to use another way around this problem: donʼt filter messages physically, but rather count the number of times a message was stopped by a smaller ID node.

So, the program now decrements the number of messages by one each time it “stops” the message. Since the message is not actually stopped, it continues to travel around the ring and forces every other node to decrement the counter of messages by one.

The final output for standard configuration of 16 processors looks like this:

lambda01:> mpirun -np 16 --hostfile hosts ./asfar
Node 0 declares node 0 a leader!
Node 1 declares node 0 a leader!
Node 5 declares node 0 a leader!
Node 3 declares node 0 a leader!
Node 13 declares node 0 a leader!
Node 9 declares node 0 a leader!
Node 7 declares node 0 a leader!
Node 11 declares node 0 a leader!
Node 2 declares node 0 a leader!
Node 15 declares node 0 a leader!
Node 6 declares node 0 a leader!
Node 10 declares node 0 a leader!
Node 14 declares node 0 a leader!
Node 4 declares node 0 a leader!
Node 8 declares node 0 a leader!
Node 12 declares node 0 a leader!
Time: 0.00546122
Messages: 68
Saved messages: 67

This program shows how modifying the first protocol can lead to more efficient use of messages, but still lacks the advantage in time, since the same number of messages is still being sent. This is where the option of adding fake messages to protocol 1 comes handy: knowing the number of saved messages in “As-Far-As-Can” protocol, we can add that number of messages to “All-The-Way” execution and check the time difference. The idea is the following: is we cannot check how second protocol is faster with k saved messages, we will check how first protocol is worse with k additional messages.

lambda01:> mpirun -np 16 --hostfile hosts ./alltheway 67
Node 15 declares node 0 a leader!
Node 11 declares node 0 a leader!
...
Node 12 declares node 0 a leader!
Elapsed time: 0.022747 seconds
Total messages sent and received: 136
Additional messages sent: 67

We can see the results clearer now:

All The Way with no additional messages: 0.020 seconds
All The Way with 67 additional messages: 0.023 seconds
As Far As Can with 67 virtual saved messages: 0.0055 seconds

## PROTOCOL “STAGES”

This protocol uses the powerful notion of proceeding to elect a leader in stages, rather than making a single attempt. Each node sends its ID to both neighbors. When a node receives values from both sides, it compares them to its own ID. If at least one of two values is smaller than its ID, a node becomes a “relayer” – a defeated entity whoʼs function from now on is only to forward any messages from left to right and from right to left. If nodeʼs ID is smaller than both received values, the node survives the stage. When a node received its own ID from both sides it declared himself a leader.

Each node has two variables for incoming messages and one variable for ID to send. In the main while loop, each entity sends its ID to left, receives from right, then sends to right and receives from left.

If a node was a “relayer”, it only forwards all incoming messages. If a node was not a “relayer”, then it either becomes one, survives to the next stage or becomes a leader.

The final output of this program looks like this:

lambda01:~/4001> mpirun -np 16 --hostfile hosts ./stages
Node 0 elected as a leader!
Elapsed time: 0.416606 seconds

## RESULTS

Legend:
• ATW = time of All The Way
• ATW, msg = number of messages used by ATW
• ATW, saved = time of ATW with respectful number of additional messages from AFAC
• ATW+k = All The Way + k additional messages
• AFAC=timeofAsFarAsCan
• AFAC, msg = number of messages used by AFAC
• AFAC, saved = number of saved messages by AFAC
• STG = time of Stages

![]([https://raw.github.com/freetonik/MPI-leader-election/master/images/4.png]

We can clearly see how AFAC is faster, than ATW+saved, and how AFAC saves us almost a half of messages by filtering them upon receiving. Yet, sometimes not so obvious results occur, for example, ATW+saved executes faster, than ATW without any additional messages. This happens because the network is used by other processes too, and the result depends on many factors within that particular network; so this measurement is only an approximation, but it gives a flavor of real-world result.