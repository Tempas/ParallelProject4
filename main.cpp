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
  int timesteps = 4;
  int NUMBER_OF_UNITS_PER_GRID = 5;

  MPI_Init(NULL, NULL);
  // get # of procs from communicator
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // get my id from the communicator
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


  int loggingRank = atoi(argv[1]);

  if (my_rank == 0)
  {
    StoplightGrid stopLightGrid(0, 3,4, 2, 1);

    int grid1Ready = 0;
    int grid2Ready = 0;
    int grid3Ready = 0;
    int grid4Ready = 0;
    MPI_Request request1;
    MPI_Request request2;
    MPI_Request request3;
    MPI_Request request4;

    //std::cout <<"Stop light waiting for 4 MpiTagBeginTimeStep"<<endl;
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

    for(int j = 0; j < timesteps; j++)
    {
      
      MPI_Send(&buffer, 1, MPI_LONG, 1, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 2, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 3, MpiTagBeginTimeStep, MPI_COMM_WORLD);
      MPI_Send(&buffer, 1, MPI_LONG, 4, MpiTagBeginTimeStep, MPI_COMM_WORLD);

      if (loggingRank == my_rank)
      {
        std::cout << std::endl <<  "***beginning timestep: " << j << " on stoplight***" <<std::endl << std::endl;
        stopLightGrid.printCars();
      }

      //std::cout <<"Stop light received 4 MpiTagBeginTimeSteps"<<endl;


// ************ Recieve Open spot request ****************
      int recievedRequestFromLeft = 0;
      int recievedRequestFromTop = 0;

      MPI_Request requestTop;
      MPI_Request requestLeft;


      MPI_Irecv(&result, 1, MPI_LONG, 2, MpiTagRequestOpenSpot, MPI_COMM_WORLD, &requestTop);
      MPI_Irecv(&result, 1, MPI_LONG, 1, MpiTagRequestOpenSpot, MPI_COMM_WORLD, &requestLeft);

      while (!recievedRequestFromLeft || !recievedRequestFromTop)
      {
        if (!recievedRequestFromTop)
        {
          MPI_Test(&requestTop, &recievedRequestFromTop, NULL);
          if (recievedRequestFromTop)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved request from top for space" << std::endl;
            int canTakeCar = 0;
            if (stopLightGrid.canAcceptNewCar(GridDirection::GridDirectionDown))
            {
              canTakeCar = 1;
            }
            MPI_Send(&canTakeCar, 1, MPI_LONG, 2, MpiTagHasOpenSpot, MPI_COMM_WORLD);
          }
        }

        if (!recievedRequestFromLeft)
        {
          MPI_Test(&requestLeft, &recievedRequestFromLeft, NULL);
          if (recievedRequestFromLeft)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved request from Left for space" << std::endl;
            int canTakeCar = 0;
            if (stopLightGrid.canAcceptNewCar(GridDirection::GridDirectionRight))
            {
              canTakeCar = 1;
            }
            MPI_Send(&canTakeCar, 1, MPI_LONG, 1, MpiTagHasOpenSpot, MPI_COMM_WORLD);
          }
        }
      }

// ************ Recieve Sending Car ****************
      recievedRequestFromLeft = 0;
      recievedRequestFromTop = 0;

      long numberOfCarsFromTop;
      long numberOfCarsFromLeft;

      MPI_Irecv(&numberOfCarsFromTop, 1, MPI_LONG, 2, MpiTagSendNumberOfCars, MPI_COMM_WORLD, &request1);
      MPI_Irecv(&numberOfCarsFromLeft, 1, MPI_LONG, 1, MpiTagSendNumberOfCars, MPI_COMM_WORLD, &request2);

      while (!recievedRequestFromLeft || !recievedRequestFromTop)
      {
        if (!recievedRequestFromTop)
        {
          MPI_Test(&request1, &recievedRequestFromTop, NULL);
          if (recievedRequestFromTop)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved number of cars from top for space" << std::endl;
            
            if (numberOfCarsFromTop == 1)
            {
              stopLightGrid.insertCar();
            }
          }
        }
        else if (!recievedRequestFromLeft)
        {
          MPI_Test(&request2, &recievedRequestFromLeft, NULL);
          if (recievedRequestFromLeft)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved number of cars from Left for space" << std::endl;
            
            if (numberOfCarsFromLeft == 1)
            {
              stopLightGrid.insertCar();
            }
          }
        }
      }

      long numOfCarsToSend;
      if (stopLightGrid.canReleaseCarAtEndOfTimeStamp())
      {
        numOfCarsToSend = 0;
        if (loggingRank == my_rank)
          std::cout << "send open spot request" << std::endl;
        MPI_Send(&result, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD);
        MPI_Recv(&result, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagHasOpenSpot, MPI_COMM_WORLD, &status);


        if (loggingRank == my_rank)
          std::cout << "Recieved has open spot request" << std::endl;
        if (result == 1)
        {
          numOfCarsToSend = 1;
          stopLightGrid.releaseFrontCar();
        }
        else
        {
          if (loggingRank == my_rank)
            std::cout << "***** ERROR - Must be able to send car from intersection" << std::endl;
        }
      }
      else
      {
        if (loggingRank == my_rank)
          std::cout << "No Cars to send" << std::endl;
        numOfCarsToSend = 0;
      }

      long carsToSendOffRoute = 0;
      MPI_Send(&numOfCarsToSend, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);
      MPI_Send(&carsToSendOffRoute, 1, MPI_LONG, stopLightGrid.GetOffForwardNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);

      stopLightGrid.increaseTimeStep();

      grid1Ready = 0;
      grid2Ready = 0;
      grid3Ready = 0;
      grid4Ready = 0;
      MPI_Request requestGrid1;
      MPI_Request requestGrid2;
      MPI_Request requestGrid3;
      MPI_Request requestGrid4;

      if (loggingRank == my_rank)
        std::cout <<"Stop light waiting for all 4 grids"<<endl;
      MPI_Irecv(&result, 1, MPI_LONG, 1, MpiTagBeginTimeStep, MPI_COMM_WORLD, &requestGrid1);
      MPI_Irecv(&result, 1, MPI_LONG, 2, MpiTagBeginTimeStep, MPI_COMM_WORLD, &requestGrid2);
      MPI_Irecv(&result, 1, MPI_LONG, 3, MpiTagBeginTimeStep, MPI_COMM_WORLD,  &requestGrid3);
      MPI_Irecv(&result, 1, MPI_LONG, 4, MpiTagBeginTimeStep, MPI_COMM_WORLD,  &requestGrid4);

      while (!grid1Ready || !grid2Ready || !grid3Ready || !grid4Ready)
      {
        if (!grid1Ready)
        {
          MPI_Test(&requestGrid1, &grid1Ready, NULL);
          if (grid1Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 1 Ready for next timestep" << std::endl;
          }
        }

        if (!grid2Ready)
        {
          MPI_Test(&requestGrid2, &grid2Ready, NULL);
          if (grid2Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 2 Ready for next timestep" << std::endl;
          }
        }

        if (!grid3Ready)
        {
          MPI_Test(&requestGrid3, &grid3Ready, NULL);
          if (grid3Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 3 Ready for next timestep" << std::endl;
          }
        }

        if (!grid4Ready)
        {
          MPI_Test(&requestGrid4, &grid4Ready, NULL);
          if (grid4Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 4 Ready for next timestep" << std::endl;
          }
        }
      }
    }   

  }      
  else
  {
    int forwardId;
    int reverseId;
    GridDirection direction;

    if(my_rank == 1)
    {
      forwardId = 0;
      reverseId = 3;
      direction = GridDirectionRight;
    }
    else if(my_rank == 2)
    {
      forwardId = 0;
      reverseId = 4;
      direction = GridDirectionDown;
    }
    else if(my_rank == 3)
    {
      forwardId = 1;
      reverseId = 0;
      direction = GridDirectionRight;
    }
    else if(my_rank == 4)
    {
      forwardId = 2;
      reverseId = 0;
      direction = GridDirectionDown;
    }

    Grid grid(NUMBER_OF_UNITS_PER_GRID, 4, my_rank, forwardId, reverseId, direction);

    MPI_Send(&buffer, 1, MPI_LONG, 0, MpiTagBeginTimeStep, MPI_COMM_WORLD);
    //std::cout <<"Grid "<<my_rank<<" sending MpiTagBeginTimeStep"<<endl;
    
    for(int i = 0; i < timesteps; i++){

      if (loggingRank == my_rank)
      {
        std::cout << "Awating to recieve next timestep" << std::endl; 
      }
      MPI_Recv(&result, 1, MPI_LONG, 0, MpiTagBeginTimeStep, MPI_COMM_WORLD, &status);
      if (loggingRank == my_rank)
      {
        std::cout << std::endl << "***Beginning timestep: " << i << " on Grid: " << my_rank << "****" << std::endl;
        grid.printCars();
      }
      int numOfCarsToSend;
      if(grid.canReleaseCarAtEndOfTimeStamp())
      {
        if (loggingRank == my_rank)
          std::cout << "Can release car - sending request" << std::endl;
        buffer = grid.GetDirection();
        MPI_Send(&buffer, 1, MPI_LONG, grid.GetForwardNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD);
        MPI_Recv(&result, 1, MPI_LONG, grid.GetForwardNeighborId(), MpiTagHasOpenSpot, MPI_COMM_WORLD, &status);
        if (loggingRank == my_rank)
          std::cout << "Recieved Response for open spot" << std::endl;
        numOfCarsToSend = result;
        if(numOfCarsToSend == 1){
          grid.releaseFrontCar();
        }
      }
      else
      {
        numOfCarsToSend = 0;
      }
      if (loggingRank == my_rank)
          std::cout << "Sending " << numOfCarsToSend << " Cars" << std::endl;
      MPI_Send(&numOfCarsToSend, 1, MPI_LONG, grid.GetForwardNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);
      grid.finishTimeStep();

      int recievedOpenSpotRequest = 0;
      int recievedNumCarsRequest = 0;
      MPI_Request request1;
      MPI_Request request2;

      if (grid.canAcceptNewCar(GridDirection::GridDirectionNone))
      {
        numOfCarsToSend = 1;
      }
      else
      {
        numOfCarsToSend = 0;
      }

      if (loggingRank == my_rank)
        std::cout << "Awaiting to recieve requests from reverse neighbor" << std::endl;
      MPI_Irecv(&result, 1, MPI_LONG, reverseId, MpiTagRequestOpenSpot, MPI_COMM_WORLD, &request1);
      MPI_Irecv(&result, 1, MPI_LONG, reverseId, MpiTagSendNumberOfCars, MPI_COMM_WORLD, &request2);      

      while (!recievedOpenSpotRequest || !recievedNumCarsRequest)
      {
        if (!recievedNumCarsRequest)
        {
          MPI_Test(&request2, &recievedNumCarsRequest, NULL);
          if (recievedNumCarsRequest)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved cars" << std::endl;
            if (result == 1)
            {
              grid.insertCar(GridDirection::GridDirectionNone);
            }
            break;
          }
        }
        if (!recievedOpenSpotRequest)
        {
          MPI_Test(&request1, &recievedOpenSpotRequest, NULL);
          if (recievedOpenSpotRequest)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved open spot request - sending cars" << std::endl;
            MPI_Send(&numOfCarsToSend, 1, MPI_LONG, reverseId, MpiTagHasOpenSpot, MPI_COMM_WORLD);
          }
        }
      }

      grid.increaseTimeStep();
      MPI_Send(&result, 1, MPI_LONG, 0, MpiTagBeginTimeStep, MPI_COMM_WORLD);
    }
  }      

  MPI_Finalize();

  std::cout << "Simulation Complete" << std::endl;

  
}