#include "mpi.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

int main(int argc, char* argv[])
{
	int size, rank, tag, rc, min, lcounter, lmc, llmc, dummy;	
	double t1, t2; 
	int inmsg[2];	//incoming msg
	int msg[2]; 	//outgoing msg
	MPI_Status Stat;
	rc=MPI_Init(&argc,&argv);
	bool terminated=false;
	if (rc!=0) {cout << "Error starting MPI." << endl; MPI_Abort(MPI_COMM_WORLD, rc);}
	MPI_Comm_size(MPI_COMM_WORLD, &size);	//size of world
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//my ID

	t1 = MPI_Wtime(); 
	msg[0]=rank;
	msg[1]=0;
	min=rank; 	//initially the minimum ID is node's own ID
	lcounter=0;	//local counter of received messages (for protocol)
	lmc=0;		//local counter of send and received messages (for analysis)
	llmc=0;
	dummy=1;
	//send my ID with counter to the right
	MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
	lmc=lmc+1; 		//first message sent	
	while (!terminated){
		//receive ID with counter from left
		MPI_Recv(&inmsg, 2, MPI_INT, (rank-1)%size, 1, MPI_COMM_WORLD, &Stat);
		lmc=lmc+1;		
		lcounter=lcounter+1;	//increment local counter
		if (min>inmsg[0]) min=inmsg[0];	//update minimum value if needed
		inmsg[1]=inmsg[1]+1;	//increment the counter before forwarding

		if (inmsg[0]==rank && lcounter==(inmsg[1])) {
			cout << "Node " << rank << " declares node " << min << " a leader!" << endl;	
			terminated=true;
		}
	
		//forward received ID
		if (inmsg[0]>min) { lmc=lmc-1; llmc=llmc+1;} 	
		MPI_Send(&inmsg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
			
	}
	if (argc>1) {
		int corr=atoi(argv[1]);
		if (rank==1)
			for (int j=0; j<corr; j++)
				MPI_Send(&dummy, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		if (rank==0)
			for (int j=0; j<corr; j++)
				MPI_Recv(&dummy, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &Stat);

	
	}
	
	t2 = MPI_Wtime(); 

	int mess;
	int saved_mess;
	MPI_Reduce(&lmc, &mess, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&llmc, &saved_mess, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Finalize();
	if (rank==0) {
		cout << "Time: " << (t2-t1) << endl;
		cout << "Messages: " << mess/2 << endl;
		

}
}