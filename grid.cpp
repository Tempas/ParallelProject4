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
private:
	void seedCars(int);

protected:
	std::vector<bool> * m_cars;
	int m_currentTimeStep;
	int m_neighborGridId;
	int m_numberOfCars;
	int m_numberOfUnits;
	int m_gridId;
	GridDirection m_direction;

public:
	Grid () {};
	Grid ( int, int, int, int, GridDirection );
	Grid ( int );

	virtual int GetNeighborId();

	virtual bool canAcceptNewCar( GridDirection );
	virtual void insertCar( GridDirection );
	virtual bool canReleaseCarAtEndOfTimeStamp();
	virtual bool releaseFrontCar();

  GridDirection GetDirection();

	void finishTimeStep();
	virtual void increaseTimeStep();

	void printCars();
};

class RedundantCarInsert : public std::exception
{
	virtual const char* what() const throw()
    {
        return "There is already a car inserted at this location";
    }
};

Grid::Grid( int numberOfUnits, int numberOfCars, int gridId, int neighborGridId, GridDirection direction ): 
		m_neighborGridId(neighborGridId), 
		m_numberOfCars(numberOfCars),
		m_numberOfUnits(numberOfUnits),
		m_direction (direction),
		m_gridId (gridId)

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

int Grid::GetNeighborId() 
{ 
	return m_neighborGridId; 
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
	int unit = 0;
	for (auto car : *m_cars)
	{
		std::cout << "unit: " << unit << " car: " << car << std::endl;
		unit++;
	}
}


class StoplightGrid : public Grid {
private:
	int m_bottomGridId;
	int m_rightGridId;

	bool m_incommingCar;

	void setDirection();
	virtual int GetNeighborId();

public:
	StoplightGrid(){};
	StoplightGrid ( int, int, int );

	bool canAcceptNewCar( GridDirection );
	void insertCar( GridDirection );
	bool releaseFrontCar();

	void finishTimeStep();
	void increaseTimeStep();

};


StoplightGrid::StoplightGrid ( int gridId, int rightGridId, int bottomGridId ):
	Grid::Grid(gridId), m_rightGridId(rightGridId), m_bottomGridId(bottomGridId)
{
	m_incommingCar = false;
	this->setDirection();
}

int StoplightGrid::GetNeighborId() 
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

void StoplightGrid::setDirection()
{
	if (m_currentTimeStep % 2 == 0)
	{
		m_direction = GridDirectionRight;
	}
	else
	{
		m_direction = GridDirectionDown;
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

void StoplightGrid::insertCar( GridDirection direction )
{
	if (this->canAcceptNewCar(direction))
	{
		m_incommingCar = true;
	}
	else
	{
		m_incommingCar = false;
	}

}


bool StoplightGrid::releaseFrontCar()
{
	if (this->canReleaseCarAtEndOfTimeStamp())
	{
		(*m_cars)[0] = m_incommingCar;
		m_numberOfCars = m_incommingCar ? 1 : 0;
		m_incommingCar = false;
		return true;
	}
	else
	{
		return false;
	}
}

void StoplightGrid::increaseTimeStep()
{
	Grid::increaseTimeStep();
	this->setDirection();
}












