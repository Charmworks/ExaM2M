#ifndef MeshData_h
#define MeshData_h

#include "Partitioner.hpp"
#include "NoWarning/mapper.decl.h"

namespace exam2m{
  struct MeshData {
    int m_nchare;                        //!< Number of worker chares
    std::size_t m_firstchunk;            //!< First chunk ID (for collision)
    CProxy_Partitioner m_partitioner;    //!< Partitioner nodegroup proxy
    tk::CProxy_MeshWriter m_meshwriter;  //!< Mesh writer nodegroup proxy
    CProxy_Mapper m_mapper;              //!< Mapper array proxy
    CProxy_Worker m_worker;              //!< Worker array proxy
    CProxy_WorkerStats m_workerStats;    //!< Worker stats array proxy
    std::size_t m_nelem;                 //!< Total number of elements in mesh
    std::size_t m_npoin;                 //!< Total number of nodes in mesh
    void pup( PUP::er& p ) {
      p | m_nchare;
      p | m_firstchunk;
      p | m_partitioner;
      p | m_meshwriter;
      p | m_mapper;
      p | m_worker;
      p | m_workerStats;
      p | m_nelem;
      p | m_npoin;
    }
    friend void operator|( PUP::er& p, MeshData& t ) { t.pup(p); }
  };
}

#endif // MeshData_h
