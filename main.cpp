#include "grid.cpp"
#include <iostream>
using std::cout; using std::endl; 
#include <mpi.h>

static int MpiTagEndTimeStep = 0;
static int MpiTagBeginTimeStep = 1;
static int MpiTagRequestOpenSpot = 2;
static int MpiTagSendNumberOfCars = 3;
static int MpiTagHasOpenSpot = 4;


int main(int argc, char **argv)
{
  const size_t sz = 10; 
  long buffer, result;
  MPI_Status status; 
  int comm_sz; // total procs 
  int my_rank; // my id 
  int timesteps = 10;
  int NUMBER_OF_UNITS_PER_GRID = 5;

  MPI_Init(&argc, &argv);
  // get # of procs from communicator
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // get my id from the communicator
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0)
  {
    StoplightGrid stopLightGrid(0, 3,4);

    for(int j = 0; j < timesteps; j++){
      int grid1Ready = 0;
      int grid2Ready = 0;
      int grid3Ready = 0;
      int grid4Ready = 0;
      MPI_Request request1;
      MPI_Request request2;
      MPI_Request request3;
      MPI_Request request4;

      std::cout <<"Stop light waiting for 4 MpiTagBeginTimeStep"<<endl;
      MPI_Irecv(&result, 1, MPI_LONG, 1, MpiTagBeginTimeStep, MPI_COMM_WORLD, &request1);
      MPI_Irecv(&result, 1, MPI_LONG, 2, MpiTagBeginTimeStep, MPI_COMM_WORLD, &request2);
      MPI_Irecv(&result, 1, MPI_LONG, 3, MpiTagBeginTimeStep, MPI_COMM_WORLD,  &request3);
      MPI_Irecv(&result, 1, MPI_LONG, 4, MpiTagBeginTimeStep, MPI_COMM_WORLD,  &request4);
      

      while (!grid1Ready || !grid2Ready || !grid3Ready || !grid4Ready)
      {
        MPI_Test(&request1, &grid1Ready, NULL);
        MPI_Test(&request2, &grid2Ready, NULL);
        MPI_Test(&request3, &grid3Ready, NULL);
        MPI_Test(&request4, &grid4Ready, NULL);
      }
      MPI_Send(&buffer, 1, MPI_LONG, 1, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 2, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 3, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 4, MpiTagBeginTimeStep, MPI_COMM_WORLD);

      std::cout <<"Stop light received 4 MpiTagBeginTimeSteps"<<endl;
    }   

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
    else if(my_rank == 2)
    {
      neighborId = 0;
      direction = GridDirectionDown;
    }
    else if(my_rank == 3)
    {
      neighborId = 1;
      direction = GridDirectionRight;
    }
    else if(my_rank == 4)
    {
      neighborId = 2;
      direction = GridDirectionDown;
    }

    Grid grid(NUMBER_OF_UNITS_PER_GRID, 4, my_rank, neighborId, direction);

    MPI_Send(&buffer, 1, MPI_LONG, 0, MpiTagBeginTimeStep, MPI_COMM_WORLD);
    std::cout <<"Grid "<<my_rank<<" sending MpiTagBeginTimeStep"<<endl;
    

    for(int i = 0; i < timesteps; i++){
      MPI_Recv(&result, 1, MPI_LONG, 1, MpiTagBeginTimeStep, MPI_COMM_WORLD, &status);
      int numOfCarsToSend;
      if(grid.canReleaseCarAtEndOfTimeStamp())
      {
        buffer = grid.GetDirection();
        MPI_Send(&buffer, 1, MPI_LONG, grid.GetNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD);
        MPI_Recv(&result, 1, MPI_LONG, grid.GetNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD, &status);
        numOfCarsToSend = result;
        if(numOfCarsToSend == 1){
          grid.releaseFrontCar();
        }
      }
      else
      {
        numOfCarsToSend = 0;
      }
      MPI_Send(&numOfCarsToSend, 1, MPI_LONG, grid.GetNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);
      grid.finishTimeStep();

      int recievedOpenSpotRequest = 0;
      int recievedNumCarsRequest = 0;
      MPI_Request request1;
      MPI_Request request2;

      MPI_Irecv(&result, 1, MPI_LONG, MPI_ANY_SOURCE, MpiTagHasOpenSpot, MPI_COMM_WORLD, &request1);
      MPI_Irecv(&result, 1, MPI_LONG, MPI_ANY_SOURCE, MpiTagSendNumberOfCars, MPI_COMM_WORLD, &request2);      

      while (!recievedOpenSpotRequest || !recievedNumCarsRequest)
      {
        MPI_Test(&request1, &recievedOpenSpotRequest, NULL);
        MPI_Test(&request2, &recievedNumCarsRequest, NULL);
        if (recievedNumCarsRequest)
        {
          break;
        }
        else if (recievedOpenSpotRequest)
        {
          MPI_Send(&numOfCarsToSend, 1, MPI_LONG, grid.GetNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);
        }
      }
    }
  }      

  MPI_Finalize();

  
}