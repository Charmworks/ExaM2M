// Controller for the library

#include "NoWarning/controller.decl.h"

#include "collidecharm.h"
#include "Fields.hpp"

namespace exam2m {

void addMesh(CkArrayID p, int elem, CkCallback cb);
void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords, const tk::Fields& u);
void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords, const tk::Fields& u, CkCallback cb);

class LibMain : public CBase_LibMain {
public:
  LibMain(CkArgMsg* msg);
};

class MeshData {
  public:
  CProxy_Worker m_proxy;
  int m_firstchunk;
  int m_nchare;
  void pup(PUP::er& p) {
    p | m_proxy;
    p | m_firstchunk;
    p | m_nchare;
  }
};

class Controller : public CBase_Controller {
  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
  #endif
  Controller_SDAG_CODE
  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic pop
  #endif

  private:
    std::unordered_map<CmiUInt8, MeshData> proxyMap;
    int current_chunk;
    CmiUInt8 m_sourcemesh, m_destmesh;

  public:
    Controller();
    void broadcastMesh(CkArrayID p, MeshData d, CkCallback cb);
    void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel,
                       tk::UnsMesh::Coords* coords, const tk::Fields& u);
    void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords,
                       const tk::Fields& u, CkCallback cb);
    void distributeCollisions(int nColl, Collision* colls);
};

}
