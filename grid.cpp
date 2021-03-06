#include <iostream>
#include <random>
#include <exception>


static int seed = 45;

enum  GridDirection{GridDirectionLeft, 
					GridDirectionRight, 
					GridDirectionUp, 
					GridDirectionDown,
					GridDirectionNone };


class Grid {
protected:
	std::vector<bool> * m_cars;
	int m_gridId;
	int m_numberOfUnits;
	int m_numberOfCars;
	int m_forwardNeighborId;
	int m_reverseNeighborId;
	int m_currentTimeStep;
	GridDirection m_direction;

	void seedCars(int);

public:
	Grid () {};
	Grid ( int, int, int, int, int, GridDirection );
	Grid ( int );

	virtual int GetForwardNeighborId();
	virtual int GetReverseNeighborId();

	virtual bool canAcceptNewCar( GridDirection );
	virtual void insertCar( GridDirection );
	virtual bool canReleaseCarAtEndOfTimeStamp();
	virtual bool releaseFrontCar();

  GridDirection GetDirection();

	virtual void finishTimeStep();
	virtual void increaseTimeStep();

	virtual void printCars();
};

class RedundantCarInsert : public std::exception
{
	virtual const char* what() const throw()
    {
        return "There is already a car inserted at this location";
    }
};

Grid::Grid( int numberOfUnits, int numberOfCars, int gridId, int forwardId, int reverseId, GridDirection direction ): 
		m_numberOfUnits(numberOfUnits),
		m_numberOfCars(numberOfCars),
		m_gridId (gridId),
		m_forwardNeighborId(forwardId), 
		m_reverseNeighborId(reverseId),
		m_direction (direction)
{
	m_currentTimeStep = 0;
	

	this->seedCars(seed);
}

Grid::Grid ( int gridId ): m_gridId(gridId)
{
	m_numberOfCars = 0;
	m_numberOfUnits = 1;
	m_direction = GridDirectionNone;
	m_currentTimeStep = 0;

	this->seedCars(seed);
}

int Grid::GetForwardNeighborId() 
{ 
	return m_forwardNeighborId; 
}

int Grid::GetReverseNeighborId()
{
	return m_reverseNeighborId;
}

GridDirection Grid::GetDirection()
{
  return m_direction;
}

void Grid::seedCars( int seed )
{
	std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> distribution(0, m_numberOfUnits -1);
    m_cars = new std::vector<bool>(m_numberOfUnits);

    int numberOfSeededCars = 0;

    while (numberOfSeededCars < m_numberOfCars)
    {
    	int insertIndex = distribution(generator);
    	if ((*m_cars)[insertIndex] == true)
    	{
    		continue;
    	}
    	else
    	{
    		(*m_cars)[insertIndex] = true;
    		numberOfSeededCars++;
    	}
    }
}

/*
 * Car addition and subtraction logic
 */

bool Grid::canAcceptNewCar (GridDirection incomingDirection)
{
	return (*m_cars)[m_numberOfUnits - 1] == false;
}

void Grid::insertCar(GridDirection incomingDirection)
{
	if (!this->canAcceptNewCar(incomingDirection))
	{
		throw RedundantCarInsert();
	}

	(*m_cars)[m_numberOfUnits - 1] = true;
	m_numberOfCars++;
}

bool Grid::canReleaseCarAtEndOfTimeStamp()
{
	return (*m_cars)[0] == true;
}

bool Grid::releaseFrontCar()
{
	if (this->canReleaseCarAtEndOfTimeStamp())
	{
		(*m_cars)[0] = false;
		m_numberOfCars--;
		return true;
	}
	else
	{
		return false;
	}
}

/*
 *  TimeStep Logic
 */

void Grid::finishTimeStep()
{
	for (auto i = 0; i < m_numberOfUnits - 1; i++)
	{
		if ((*m_cars)[i+1] == true && (*m_cars)[i] == false)
		{
			(*m_cars)[i] = true;
			(*m_cars)[i+1] = false;
		}
	}
}

void Grid::increaseTimeStep()
{
	m_currentTimeStep++;
}

void Grid::printCars()
{
	std::cout << std::endl;
	std::cout << "Grid: " << m_gridId << std::endl;
	int unit = 0;
	for (auto car : *m_cars)
	{
		std::cout << "unit: " << unit << " car: " << car << std::endl;
		unit++;
	}

	std::cout << std::endl;
}


class StoplightGrid : public Grid {
protected:
	int m_gridId;
private:
	int m_bottomGridId;
	int m_rightGridId;
	int m_topGridId;
	int m_leftGridId;

	bool m_bottomGoingCar;
  bool m_rightGoingCar;

	void setDirection();


public:
	StoplightGrid(){};
	StoplightGrid ( int, int, int, int, int );

	bool canAcceptNewCar( GridDirection );
	void insertCar ();
	bool releaseFrontCar();
  bool HasBottomGoingCar();
  bool HasRightGoingCar();

	void increaseTimeStep();

	int GetForwardNeighborId();
	int GetReverseNeighborId();
	int GetOffForwardNeighborId();

	void printCars();

};


StoplightGrid::StoplightGrid ( int gridId, int rightGridId, int bottomGridId, int topGridId, int leftGridId ):
	 m_rightGridId(rightGridId), m_bottomGridId(bottomGridId), m_leftGridId(leftGridId),m_topGridId(topGridId)
{
	m_gridId = 0;
	m_numberOfCars = 0;
	m_numberOfUnits = 1;
	m_direction = GridDirectionNone;
	m_currentTimeStep = 0;
	Grid::seedCars(m_gridId);
	this->setDirection();
}

int StoplightGrid::GetForwardNeighborId()
{
	if (m_direction == GridDirectionRight)
	{
		return m_rightGridId;
	}
	else
	{
		return m_bottomGridId;
	}
}

int StoplightGrid::GetOffForwardNeighborId()
{
	if (m_direction == GridDirectionRight)
	{
		return m_bottomGridId;
	}
	else
	{
		return m_rightGridId;
	}
}

int StoplightGrid::GetReverseNeighborId()
{
	if (m_direction == GridDirectionRight)
	{
		return m_leftGridId;
	}
	else
	{
		return m_topGridId;
	}
}

void StoplightGrid::setDirection()
{
	if (m_currentTimeStep % 2 == 0)
	{
		m_direction = GridDirectionDown;
	}
	else
	{
    m_direction = GridDirectionRight;
	}
}

bool StoplightGrid::canAcceptNewCar( GridDirection direction )
{
	if ( direction == m_direction )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void StoplightGrid::insertCar()
{
	(*m_cars)[0] = true;
}


bool StoplightGrid::releaseFrontCar()
{
	(*m_cars)[0] = false;
  return true;
}

void StoplightGrid::increaseTimeStep()
{
	Grid::increaseTimeStep();
	this->setDirection();
  (*m_cars)[0] = false;
}


void StoplightGrid::printCars()
{
	if (m_direction == GridDirectionRight)
	{
		std::cout << "Direction: Right" << std::endl;
	}
	else if (m_direction == GridDirectionDown)
	{
		std::cout << "Direction: Down" << std::endl;
	}
	Grid::printCars();
}














