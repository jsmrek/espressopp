from espresso import pmi
from espresso import boostmpi as mpi
from espresso.decomposition.Decomposer import *

__all__ = [ "SingleNodeLocal" ]

class SingleNodeLocal(DecomposerLocal):
    """
    The local particle cxxobject that puts all particles on a dedicated node. So,
    this actually is nothing on most of the nodes.
    """
    def __init__(self, masternode):
        DecomposerLocal.__init__(self)
        self.masternode = masternode
    
    def addParticle(self, id):
        if mpi.rank == self.masternode :
            DecomposerLocal.addParticle(self, id)

    def deleteParticle(self, id):
        if mpi.rank == self.masternode :
            DecomposerLocal.deleteParticle(self, id)
####

if pmi.IS_CONTROLLER :
    __all__.append("SingleNode")

    pmi.exec_('from espresso.decomposition.SingleNode import SingleNodeLocal')

    class SingleNode(Decomposer) :
        """
        The single node particle storage. Stores all particles
        on a single, configurable node.
        """
        def __init__(self, node = pmi.CONTROLLER) :
            self.pmiobject = pmi.create('SingleNodeLocal', node)
            self.masternode = node
            # list of all particles. Since they are all on one node, we can as well
            # keep a table of all of them here
            self.particleIds = set()
            self.maxSeenId = -1

        def addParticle(self, id = None) :
            if id in self.particleIds :
                raise IndexError("particle %s already exists" % str(id))
            if id is None:
                id = self.maxSeenId + 1
            elif type(id) is not type(1) or id < 0 :
                raise TypeError("particle identity should be a nonnegative integer")
            pmi.call(self.pmiobject.addParticle, id)
            # update maxSeenId and list of particleIds
            if id > self.maxSeenId :
                self.maxSeenId = id
            self.particleIds.add(id)
            return id

        def deleteParticle(self, id) :
            if id not in self.particleIds :
                raise IndexError("particle %s does not exist" % str(id))
            self.particleIds.remove(id)
            pmi.call(self.pmiobject.deleteParticle, id)
        
        def getNodeOfParticle(self, id) :
            if id not in self.particleIds :
                raise IndexError("particle %s does not exist" % str(id))
            return self.masternode

        def getTotalNumberOfParticles(self) :
            return len(self.particleIds)
####
