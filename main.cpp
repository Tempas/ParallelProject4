#include "grid.cpp"
#include <iostream>
using std::cout; using std::endl; 
#include <mpi.h>


int main(int argc, char **argv)
{
  int comm_sz; // total procs 
  int my_rank; // my id 

  Grid grid(5, 4, 0, 1, GridDirectionRight);

  MPI_Init(&argc, &argv);
  // get # of procs from communicator
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // get my id from the communicator
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0)
  {
    cout <<"I'm the master process 0"<<endl;
    StoplightGrid stoplight(my_rank, 3, 4);



  }
      
  else
  {
    int neighborId;
    GridDirection direction;
    if(my_rank == 1)
    {
      neighborId = 0;
      direction = GridDirectionRight;
    }
    else if ()


    Grid grid( )
    cout <<"I'm process:"<<my_rank<<", out of:"<<comm_sz<<endl;
  }
      

  MPI_Finalize();

  grid.printCars();
  grid.finishTimeStep();
  std::cout << std::endl;
  grid.printCars();

  
}