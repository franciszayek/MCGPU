/*
   New Simulation to replace linearSim and parallelSim

   Author: Nathan Coleman
   Last Changed: February 21, 2014
   
   -> February 26, by Albert Wallace
   -> March 28, by Joshua Mosby
*/

#include <stdio.h>
#include <string>
#include <iostream>
#include "Simulation.h"
#include "SimulationArgs.h"
#include "Box.h"
#include "Metropolis/Utilities/MathLibrary.h"
#include "SerialSim/SerialBox.h"
#include "SerialSim/SerialCalcs.h"
#include "ParallelSim/ParallelCalcs.h"
#include "Utilities/FileUtilities.h"


//Constructor & Destructor
Simulation::Simulation(SimulationArgs simArgs)
{
   args = simArgs;
   std::string configpath(simArgs.configPath);

   if (simArgs.simulationMode == SimulationMode::Parallel)
      box = ParallelCalcs::createBox(configpath, &simSteps);
   else
      box = SerialCalcs::createBox(configpath, &simSteps);

   if (box == NULL)
   {
      std::cerr << "Error: Unable to initialize simulation Box" << std::endl;
      exit(EXIT_FAILURE);
   }
}

Simulation::~Simulation()
{
   if(box != NULL)
   {
      delete box;
      box = NULL;
   }
}

//Utility
void Simulation::run()
{
   //declare variables common to both parallel and serial
   Molecule *molecules = box->getMolecules();
   Environment *enviro = box->getEnvironment();
   double oldEnergy = 0, currentEnergy = 0;
   double newEnergyCont, oldEnergyCont;
   double  kT = kBoltz * enviro->temp;
   int accepted = 0;
   int rejected = 0;
   
   //calculate old energy
   if (oldEnergy == 0)
   {
      if (args.simulationMode == SimulationMode::Parallel) {
         oldEnergy = ParallelCalcs::calcSystemEnergy(box);
      }
      else {
         oldEnergy = SerialCalcs::calcSystemEnergy(molecules, enviro);
      }
   }
   
   for(int move = 0; move < simSteps; move++)
   {
      int changeIdx = box->chooseMolecule();
      
      if (args.simulationMode == SimulationMode::Parallel) {
         oldEnergyCont = ParallelCalcs::calcMolecularEnergyContribution(box, changeIdx);
      }
      else {
         oldEnergyCont = SerialCalcs::calcMolecularEnergyContribution(molecules, enviro, changeIdx);
      }
         
      box->changeMolecule(changeIdx);
      
      if (args.simulationMode == SimulationMode::Parallel) {
         newEnergyCont = ParallelCalcs::calcMolecularEnergyContribution(box, changeIdx);
      }
      else {
         newEnergyCont = SerialCalcs::calcMolecularEnergyContribution(molecules, enviro, changeIdx);
      }
      
      bool accept = false;
      
      if(newEnergyCont < oldEnergyCont)
      {
         accept = true;
      }
      else
      {
         double x = exp(-(newEnergyCont - oldEnergyCont) / kT);
         
         if(x >= randomReal(0.0, 1.0))
         {
            accept = true;
         }
         else
         {
            accept = false;
         }
      }
      
      if(accept)
      {
         accepted++;
         oldEnergy += newEnergyCont - oldEnergyCont;
         std::cout << "ACCEPTED: New Energy: " << oldEnergy << std::endl;
      }
      else
      {
         rejected++;
         //restore previous configuration
         box->rollback(changeIdx);
         std::cout << "REJECTED: Old Energy: " << oldEnergy << std::endl;
      }
   }
   currentEnergy = oldEnergy;
   
   std::cout << std::endl << std::endl;
   std::cout << "Finished running " << simSteps << " steps" << std::endl;
   std::cout << "FINAL ENERGY: " << currentEnergy << std::endl;
}


