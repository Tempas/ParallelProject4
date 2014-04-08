#include "grid.cpp"
#include <iostream>
#include <chrono>
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

  int NUMBER_OF_UNITS_PER_GRID = atoi(argv[1]);
  int NUMBER_OF_CARS_PER_GRID = atoi(argv[2]);
  int timesteps = atoi(argv[3]);
  int loggingRank = 1;

  MPI_Init(NULL, NULL);
  // get # of procs from communicator
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // get my id from the communicator
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


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

    //start time
    auto t1 = std::chrono::high_resolution_clock::now();

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

      long canTakeTopCar = 0;
      long canTakeLeftCar = 0;
      while (!recievedRequestFromLeft || !recievedRequestFromTop)
      {
        if (!recievedRequestFromTop)
        {
          MPI_Test(&requestTop, &recievedRequestFromTop, &status);
          if (recievedRequestFromTop)
          {
            if (stopLightGrid.canAcceptNewCar(GridDirection::GridDirectionDown))
            {
              canTakeTopCar = 1;
            }
            if (loggingRank == my_rank)
              std::cout << "Recieved request from top for space sending: " << canTakeTopCar << std::endl;
            MPI_Send(&canTakeTopCar, 1, MPI_LONG, 2, MpiTagHasOpenSpot, MPI_COMM_WORLD);
          }
        }

        if (!recievedRequestFromLeft)
        {
          MPI_Test(&requestLeft, &recievedRequestFromLeft, &status);
          if (recievedRequestFromLeft)
          {
            if (stopLightGrid.canAcceptNewCar(GridDirection::GridDirectionRight))
            {
              canTakeLeftCar = 1;
            }
            if (loggingRank == my_rank)
              std::cout << "Recieved request from Left for space sending: " << canTakeLeftCar << std::endl;
            MPI_Send(&canTakeLeftCar, 1, MPI_LONG, 1, MpiTagHasOpenSpot, MPI_COMM_WORLD);
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
              std::cout << "Recieved " << numberOfCarsFromTop << " cars from top for space" << std::endl;
            
            if (numberOfCarsFromTop == 1)
            {
              stopLightGrid.insertCar();
            }
          }
        }
        else if (!recievedRequestFromLeft)
        {
          MPI_Test(&request2, &recievedRequestFromLeft, &status);
          if (recievedRequestFromLeft)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved " << numberOfCarsFromLeft << " cars from Left for space" << std::endl;
            
            if (numberOfCarsFromLeft == 1)
            {
              stopLightGrid.insertCar();
            }
          }
        }
      }


      long numOfCarsToSend = 0;
      if (loggingRank == my_rank)
        std::cout << "send open spot request to Grid: " << stopLightGrid.GetForwardNeighborId() << std::endl;
      MPI_Send(&result, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD);
      if (loggingRank == my_rank) std::cout << "sent, waiting for open spot request from "<<stopLightGrid.GetForwardNeighborId() << std::endl;
      //MPI_Recv(&result, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagHasOpenSpot, MPI_COMM_WORLD, &status);
      if (loggingRank == my_rank) std::cout << "got it"<<std::endl;

      if (loggingRank == my_rank)
        std::cout << "Forward grid has " << result << " spots" << std::endl;
      
      if (stopLightGrid.canReleaseCarAtEndOfTimeStamp())
      {
        numOfCarsToSend = 1;
        stopLightGrid.releaseFrontCar();
      }


      long carsToSendOffRoute = 0;
      MPI_Send(&numOfCarsToSend, 1, MPI_LONG, stopLightGrid.GetForwardNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);
      MPI_Send(&carsToSendOffRoute, 1, MPI_LONG, stopLightGrid.GetOffForwardNeighborId(), MpiTagSendNumberOfCars, MPI_COMM_WORLD);

      stopLightGrid.finishTimeStep();
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
          MPI_Test(&requestGrid1, &grid1Ready, &status);
          if (grid1Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 1 Ready for next timestep" << std::endl;
          }
        }

        if (!grid2Ready)
        {
          MPI_Test(&requestGrid2, &grid2Ready, &status);
          if (grid2Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 2 Ready for next timestep" << std::endl;
          }
        }

        if (!grid3Ready)
        {
          MPI_Test(&requestGrid3, &grid3Ready, &status);
          if (grid3Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 3 Ready for next timestep" << std::endl;
          }
        }

        if (!grid4Ready)
        {
          MPI_Test(&requestGrid4, &grid4Ready, &status);
          if (grid4Ready)
          {
            if (loggingRank == my_rank)
              std::cout << "Grid 4 Ready for next timestep" << std::endl;
          }
        }
      }
      if(j == timesteps-1)
      {
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "Traffic simulation took "
        << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()
        << " microseconds\n";
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

    Grid grid(NUMBER_OF_UNITS_PER_GRID, NUMBER_OF_CARS_PER_GRID, my_rank, forwardId, reverseId, direction);

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

      long numOfCarsToSend = 0;
      
      if (loggingRank == my_rank)
        std::cout << "Can release car - sending request" << std::endl;
      buffer = grid.GetDirection();
      long availableSpace = 0;
      MPI_Send(&buffer, 1, MPI_LONG, grid.GetForwardNeighborId(), MpiTagRequestOpenSpot, MPI_COMM_WORLD);
      MPI_Recv(&availableSpace, 1, MPI_LONG, grid.GetForwardNeighborId(), MpiTagHasOpenSpot, MPI_COMM_WORLD, &status);
      if (loggingRank == my_rank)
        std::cout << "Recieved Response for open spot: " << availableSpace << " from grid: " << grid.GetForwardNeighborId() << std::endl;
      if (availableSpace == 0)
      {
        numOfCarsToSend = 0;
      }
      else if (grid.canReleaseCarAtEndOfTimeStamp() && availableSpace == 1)
      {
        numOfCarsToSend = 1;
      }

      if(numOfCarsToSend == 1){
        grid.releaseFrontCar();
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
        std::cout << "Awaiting to recieve requests from reverse neighbor"<<reverseId<< std::endl;
      long numCarsRecieved = 0;
      MPI_Irecv(&result, 1, MPI_LONG, reverseId, MpiTagRequestOpenSpot, MPI_COMM_WORLD, &request1);
      MPI_Irecv(&numCarsRecieved, 1, MPI_LONG, reverseId, MpiTagSendNumberOfCars, MPI_COMM_WORLD, &request2);      

      while (!recievedOpenSpotRequest || !recievedNumCarsRequest)
      {
        //std::cout << recievedOpenSpotRequest<<recievedNumCarsRequest<< std::endl;
        if (!recievedNumCarsRequest)
        {
          MPI_Test(&request2, &recievedNumCarsRequest, &status);
          if (recievedNumCarsRequest)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved " << numCarsRecieved << " cars" << std::endl;
            if (numCarsRecieved == 1)
            {
              grid.insertCar(GridDirection::GridDirectionNone);
            }
            break;
          }
        }
        if (!recievedOpenSpotRequest)
        {
          MPI_Test(&request1, &recievedOpenSpotRequest, &status);
          if (recievedOpenSpotRequest)
          {
            if (loggingRank == my_rank)
              std::cout << "Recieved open spot request - sending "<< numOfCarsToSend << " cars" << std::endl;
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