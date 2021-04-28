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
  private:
    std::unordered_map<CmiUInt8, MeshData> proxyMap;
    int current_chunk;
    CmiUInt8 m_sourcemesh, m_destmesh;

  public:
    Controller();
    #if defined(__clang__)
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wundefined-func-template"
    #endif
    explicit Controller( CkMigrateMessage* ) {}
    #if defined(__clang__)
      #pragma clang diagnostic pop
    #endif
    void addMesh(CkArrayID p, int elem, CkCallback cb);
    void setMesh(CkArrayID p, MeshData d);
    void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel,
                       tk::UnsMesh::Coords* coords, const tk::Fields& u);
    void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords,
                       const tk::Fields& u, CkCallback cb);

    void receiveSerialCollisions(CkReductionMsg* msg);
    void receiveDistributedCollisions(CkDataMsg* msg);
    void distributeCollisions(int nColl, Collision* colls);
};

}
