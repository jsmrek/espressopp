#include "log4espp.hpp"
#include "ParticleContainer.hpp"

#define DEFAULT_BLOCK_SIZE 256

#include <iostream>
#include <cstdlib>
#include <cstring>

/** \file ParticleContainer.cpp    i

Implementation of class ParticleContainer for storing particle data.

<b>Responsible:</b>
<a href="mailto:brandes@scai.fraunhofer.de">Thomas Brandes</a>

*/

LOG4ESPP_LOGGER(ParticleContainer::myLogger,"kernel.ParticleContainer");

/** This is the constructor of a ParticleContainer.
 *
 * \param numberBlocks specifies the initial number of blocks of particles
 * \param blockDefaultSize is the initial size used for a particle block
*/

ParticleContainer::ParticleContainer(int numberBlocks, int blockDefaultSize) {

   noBlocks = numberBlocks;

   blockOffset.resize(numberBlocks+1);
   blockSize.resize(numberBlocks);

   blockOffset[0] = 0;

   for (int i=0; i < numberBlocks; i++) {

       blockOffset[i+1] = blockOffset[i] + blockDefaultSize;
       blockSize[i]     = 0;
   }

   int maxParticles = blockOffset [numberBlocks];

   keyData = (int *) malloc (maxParticles * sizeof(int));

   LOG4ESPP_INFO(myLogger,"ParticeContainer for " << numberBlocks <<
                          " blocks (size of block = " << blockDefaultSize << ") created");

   LOG4ESPP_DEBUG(myLogger,"size of blockOffset = " << blockOffset.size());
   LOG4ESPP_DEBUG(myLogger,"size of blockSize   = " << blockSize.size());

   for (int i=0; i < numberBlocks; i++) {

       LOG4ESPP_DEBUG(myLogger,"block " << i << ": offset = " << blockOffset[i] <<
                               ", total size = " << blockOffset[i+1] - blockOffset[i] <<
                               ", act size = " << blockSize[i]);
   }

}

void ParticleContainer::addProperty (BasicProperty *newProperty) {

  int position = propertyList.size();

  // put the property at the end

  propertyList.push_back(newProperty);

  // property itself provies that is not registered yet

  newProperty->registerContainer(this, position);
 
  int maxNumberParticles = blockOffset [noBlocks];
  int size               = newProperty->getSize ();

  size *= maxNumberParticles;

  void *data = (void *) malloc (size);

  propertyData.push_back (data);

  LOG4ESPP_DEBUG(myLogger, "addProperty: " << size << " bytes allocated for property " <<
                           newProperty->getName() << "\n");

}

void ParticleContainer::removeProperty (BasicProperty *p) {

  if (!p->isRegistered(this)) {

     cout << "removeProperty ", p->getName(), " not registered for this container";

  }

  // get the position of the property in the propertyList

  int pos = p->getPosition();

  // remove the elements from propertyList and propertyData

  propertyList.erase (propertyList.begin()+pos);
  propertyData.erase (propertyData.begin()+pos);

  p->deregister();

}

ParticleRef ParticleContainer::addParticle (int blockIndex, int key) {

   LOG4ESPP_DEBUG(myLogger,"add particle for block " << blockIndex);
   LOG4ESPP_DEBUG(myLogger,"block offset = " << blockOffset[blockIndex]);
   LOG4ESPP_DEBUG(myLogger,"block size   = " << blockSize[blockIndex]);
 
   // we make here the proof of a legal blockIndex

   int bsize = blockSize.at(blockIndex)++;
   int index = blockOffset[blockIndex] + bsize;

   if (blockOffset[blockIndex] + bsize >= blockOffset[blockIndex+1]) {

      LOG4ESPP_FATAL(myLogger,"addParticle, memory for block " << blockIndex <<
                              " exhausted, max = " << bsize-1);
      exit(-1);

   }

   // store already the key value

   keyData [index] = key;

   return ParticleRef (index, this);
}

void* ParticleContainer::getAllParticleData(BasicProperty *prop) {

  if (!prop->isRegistered(this)) {

     cout << "getAllParticleData: " << prop->getName() << " not registered for this container\n";

  }

  void *data = propertyData[prop->getPosition()];

  if (data == NULL) {

     cout << "SERIOUS ERROR: no paraticle data\n" ;
  }

  return data;
}

void* ParticleContainer::getParticleDataPtr(ParticleRef pRef, BasicProperty *prop) {

  if (pRef.pc != this) {

     cout << "getParticeDataPtr: illegal reference, not this container\n";
     exit(-1);
  }

  char* ptr = (char *) getAllParticleData(prop);   // points to data of property

  int maxParticles = blockOffset [noBlocks];       // neeeded for range check

  int index = pRef.index;

  if ((index < 0) || (index >= maxParticles)) {

     cout << "getParticleDataPtr, pRef = " << index << " out of range 0 - " << maxParticles-1 << "\n";
     exit(-1);
  }

  ptr += index * prop->getSize ();  // add offset on data for the needed particle

  return (void *) ptr;
}

void ParticleContainer::setParticleData (ParticleRef pRef, BasicProperty *prop, void *data) {

  LOG4ESPP_DEBUG(myLogger, "setPartcileData for particle " << pRef.index);
  LOG4ESPP_DEBUG(myLogger, "prop = " << prop->getName() << " at pos = " << prop->getPosition() << "\n");

  char* ptr = (char *) getParticleDataPtr(pRef, prop);

  memcpy (ptr, data, prop->getSize());
}

void ParticleContainer::getParticleData(ParticleRef pRef, BasicProperty *prop, void *data) {

#ifdef DEBUG
  LOG4ESPP_DEBUG("getPartcileData for particle " << pRef << ", max = " << maxParticles);
  LOG4ESPP_DEBUG("prop = " << prop->getName() << " at pos = " << prop->getPosition());
#endif

  char* ptr = (char *) getParticleDataPtr(pRef, prop);

  memcpy (data, ptr, prop->getSize());
}

void ParticleContainer:: moveParticle(ParticleRef p, int blockIndex) {

   LOG4ESPP_FATAL(myLogger,"moveParticle not available yet");

}

int ParticleContainer::getParticleKey(ParticleRef pRef) {

   return keyData [pRef.index];

}

int* ParticleContainer::getAllParticleKey() {

  return keyData;
}

ParticleRef ParticleContainer::findParticle (int key) {

  // ToDo: replace this sequential search with search by hashing

  int index = -1;  // value for not found

  for (int i=0; i < noBlocks; i++) {

      int start = blockOffset[i];
      int stop  = start + blockSize[i];

      for (int j = start; j < stop; j++) {

         if (keyData[j] == key) {

            index = j;
            break;
         }

      }

      if (index >= 0) {

         break;
      }

  }

  return ParticleRef(index,this);

}

void ParticleContainer::printInfo () {

   vector <BasicProperty *> :: iterator it;

   cout << "--begin-- ParticleContainer::printInfo\n";

   for (it = propertyList.begin(); it != propertyList.end(); it++) {

      (*it)->printInfo();

   }

   printf ("number of Blocks = %d\n", noBlocks);

   for (int i=0; i < noBlocks; i++) {

       printf ("block %d : from %d - %d, has %d particles\n",
                i, blockOffset[i], blockOffset[i+1]-1, blockSize[i]);

   }

   cout << "---end--- ParticleContainer::printInfo\n";
}

ParticleIterator ParticleContainer::begin() {

   ParticleIterator first;
   first.block = 0;
   first.blockpos = -1;
   first.pref = -1;
   first.myContainer = this;
   first.nextPos();          // finds first valid particle
   return first;
}

ParticleIterator ParticleContainer::end() {

   ParticleIterator first;
   first.block =-1;
   first.blockpos =-1;
   first.myContainer = this;
   return first;
}

ParticleRef ParticleIterator::operator*() {

   // we can be sure that we have a legal position

   return ParticleRef(pref,myContainer);
}

void ParticleIterator::nextPos() {

   // increase the block position

   blockpos++;
   pref++;

   if (blockpos < myContainer->blockSize[block])
 
      return;

   // increase the block

   block++;

   while (block < myContainer->noBlocks) {

      // take the block if it has at least one element

      if (myContainer->blockSize[block] > 0) {

         pref=myContainer->blockOffset[block];
         blockpos = 0;
         return;
      }

      // try next block

      block ++;
   }

   // no block found

   block = -1;
   blockpos = -1;
   pref = -1;

}

ParticleIterator& ParticleIterator::operator++() {

   nextPos();
   return *this;
}