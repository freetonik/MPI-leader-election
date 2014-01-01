#include "mpi.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

int main(int argc, char* argv[])
{
	int size, rank, tag, rc, min, lcounter;	
	double t1, t2; 
	int inmsg1;	//incoming msg from right
	int inmsg2;	//incoming msg from left
	int msg; 	//outgoing msg
	int stage=1;	//stage of election
	MPI_Status Stat;
	MPI_Request send_req, recv_req;
	rc=MPI_Init(&argc,&argv);
	bool terminated=false;
	bool relayer=false;
	if (rc!=0) {cout << "Error starting MPI." << endl; MPI_Abort(MPI_COMM_WORLD, rc);}
	MPI_Comm_size(MPI_COMM_WORLD, &size);	//size of world
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//my ID

	t1 = MPI_Wtime(); 
	msg=rank;
	
	while (!terminated){
		//receive ID with counter from left and right
		if (rank==0){
			MPI_Isend(&msg, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &send_req);
			MPI_Recv(&inmsg1, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &Stat);
			MPI_Isend(&msg, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &send_req);
			MPI_Recv(&inmsg2, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &Stat);
		}
		else if (rank==size-1){
			MPI_Isend(&msg, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_req);
			MPI_Recv(&inmsg1, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &Stat);
			MPI_Isend(&msg, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &send_req);	
			MPI_Recv(&inmsg2, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
			
		}
		else {
			MPI_Isend(&msg, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &send_req);
			MPI_Recv(&inmsg1, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &Stat);
			MPI_Isend(&msg, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &send_req);
			MPI_Recv(&inmsg2, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &Stat);
		}
		
	//forward the message if relayer

		if (relayer) {
			if (rank==0) {
				MPI_Send(&inmsg1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
				MPI_Send(&inmsg2, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD);
			}
			else if (rank==size-1) {
				MPI_Send(&inmsg1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
				MPI_Send(&inmsg2, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
			}
			else {
				MPI_Send(&inmsg1, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
				MPI_Send(&inmsg2, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
			}		
		}

			if (!relayer){
			if (inmsg1<rank || inmsg2<rank) 
			{
				relayer=true; 	//become relayer or survive
			//	cout << "Node " << rank << " became a relayer" << endl;
			}
			else if (inmsg1==rank && inmsg2==rank) {
				terminated=true;
				cout << "Node " << rank << " elected as a leader!" << endl;
				t2 = MPI_Wtime(); 
				cout << endl << "Elapsed time: " << (t2-t1) << " seconds" << endl;
				MPI_Finalize();
				break;
			}
			}

		
			
	}
	t2 = MPI_Wtime(); 
	
MPI_Finalize();
if (rank==min) cout << endl << "Elapsed time: " << (t2-t1) << " seconds" << endl;
}