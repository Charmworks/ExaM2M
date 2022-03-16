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
    bool dest;
    void pup(PUP::er& p) {
      p | m_proxy;
      p | m_firstchunk;
      p | m_nchare;
      p | dest;
    }
};

class DetailedCollision {
public:
  std::size_t source_chunk, dest_chunk;
  std::size_t source_index, dest_index;
  // TODO: Can this just be a more generic type like std::array
  CkVector3d point;
  void pup(PUP::er& p) {
    p | source_chunk; p | source_index;
    p | dest_chunk; p | dest_index;
    p | point;
  }
};
}

namespace std {
  template <>
  struct hash<exam2m::MeshData> {
    size_t operator()(const exam2m::MeshData& m) const {
      return hash<int>()(m.m_firstchunk);
    }
  };

  template<>
  struct equal_to<exam2m::MeshData> {
    bool operator()(const exam2m::MeshData& m1, const exam2m::MeshData& m2) const {
      return equal_to<int>()(m1.m_firstchunk, m2.m_firstchunk);
    }
  };
}

namespace exam2m {

class Controller : public CBase_Controller {
  private:
    std::unordered_map<CmiUInt8, MeshData> proxyMap;
    int current_chunk;

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

    using MeshDict = std::unordered_map<MeshData, std::vector<std::vector<DetailedCollision>>>;

    void addMesh(CkArrayID p, int elem, CkCallback cb);
    void setMesh(CkArrayID p, MeshData d);
    void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel,
                       tk::UnsMesh::Coords* coords, const tk::Fields& u);
    void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords,
                       const tk::Fields& u, CkCallback cb);

    void distributeCollisions(CkDataMsg* msg) {
      distributeCollisions(msg->getSize()/sizeof(Collision), (Collision*)msg->getData());
    }
    void distributeCollisions(int nColl, Collision* colls);
    void separateCollisions(MeshDict& outgoing, bool dest, int nColl,
                            Collision* colls) const;
    void separateCollisions(MeshDict& outgoing, bool dest, int nColl,
                            DetailedCollision* colls) const;

};

}
