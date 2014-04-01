/*
	Contains calculations for ParallelBox

	Author: Nathan Coleman
*/

#ifndef PARALLELCALCS_H
#define PARALLELCALCS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include "Metropolis/Box.h"
#include "Metropolis/DataTypes.h"

namespace ParallelCalcs
{
	Box* createBox(std::string configpath, long* steps);
	Real calcSystemEnergy(Box *box);
	Real calcMolecularEnergyContribution(Box *box, int molIdx, int startIdx = 0);
}

#endif