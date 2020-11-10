// Controller for the library

#include "NoWarning/controller.decl.h"

#include "collidecharm.h"

namespace exam2m {

void addMesh(CkArrayID p, int elem, CkCallback cb);
void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords, std::vector< tk::real >* u);
void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords, std::vector< tk::real >* u, CkCallback cb);

class LibMain : public CBase_LibMain {
public:
  LibMain(CkArgMsg* msg);
};

struct MeshData {
  CProxy_Worker m_proxy;
  int m_firstchunk;
  int m_nchare;
  virtual void pup(PUP::er& p) {
    p | m_proxy;
    p | m_firstchunk;
    p | m_nchare;
  }
};

class Controller : public CBase_Controller {
Controller_SDAG_CODE;
private:
  std::unordered_map<CmiUInt8, MeshData> proxyMap;
  int current_chunk;
  CmiUInt8 m_sourcemesh, m_destmesh;

public:
  Controller();
  void broadcastMesh(CkArrayID p, MeshData d, CkCallback cb);
  void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords, std::vector< tk::real >* u);
  void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords, std::vector< tk::real >* u, CkCallback cb);
  void distributeCollisions(int nColl, Collision* colls);
};

}
