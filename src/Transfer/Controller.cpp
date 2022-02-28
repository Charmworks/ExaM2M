// Controller for the library

#include "Controller.hpp"
#include "Worker.hpp"

#include <cassert>

namespace exam2m {

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-prototypes"
  #pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

/* readonly */ CProxy_Controller controllerProxy;
//! \brief Charm handle to the collision detection library instance
/* readonly */ CollideHandle collideHandle;

void collisionHandler( [[maybe_unused]] void *param,
                        int nColl,
                        Collision *colls )
{
  controllerProxy.ckLocalBranch()->distributeCollisions( nColl, colls );
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
  #pragma clang diagnostic ignored "-Wunused-private-field"
  #pragma clang diagnostic ignored "-Wvla"
  #pragma clang diagnostic ignored "-Wvla-extension"
#endif

void addMesh(CkArrayID p, int elem, CkCallback cb) {
  controllerProxy[0].addMesh(p, elem, cb);
}

void setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords, const tk::Fields& u) {
  controllerProxy.ckLocalBranch()->setSourceTets(p, index, inpoel, coords, u);
}

void setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords, const tk::Fields& u, CkCallback cb) {
  controllerProxy.ckLocalBranch()->setDestPoints(p, index, coords, u, cb);
}

LibMain::LibMain(CkArgMsg* msg) {
  delete msg;
  controllerProxy = CProxy_Controller::ckNew();

  // TODO: Need to make sure this is actually correct
  CollideGrid3d gridMap(CkVector3d(0, 0, 0),CkVector3d(2, 100, 2));
  collideHandle = CollideCreate(gridMap,
      CollideSerialClient(collisionHandler, 0));
}

Controller::Controller() : current_chunk(0) {}

void Controller::addMesh(CkArrayID p, int elem, CkCallback cb) {
  auto id = static_cast<std::size_t>(CkGroupID(p).idx);
  if (proxyMap.count(id) == 0) {
    CkArrayOptions opts;
    opts.bindTo(p);
    opts.setNumInitial(elem);
    MeshData mesh;
    mesh.m_nchare = elem;
    mesh.m_firstchunk = current_chunk;
    mesh.m_proxy = CProxy_Worker::ckNew(p, mesh, cb, opts);
    proxyMap[id] = mesh;
    current_chunk += elem;
  } else {
    CkAbort("Uhoh...\n");
  }
}
void Controller::setMesh( CkArrayID p, MeshData d ) {
  proxyMap[static_cast<std::size_t>(CkGroupID(p).idx)] = d;
}

void Controller::setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords, const tk::Fields& u, CkCallback cb) {
  proxyMap[CkGroupID(p).idx].dest = true;
  Worker* w = proxyMap[CkGroupID(p).idx].m_proxy[index].ckLocal();
  assert(w);
  w->setDestPoints(coords, u, cb);
}

void Controller::setSourceTets(CkArrayID p, int index, std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords, const tk::Fields& u) {
  proxyMap[CkGroupID(p).idx].dest = false;
  Worker* w = proxyMap[CkGroupID(p).idx].m_proxy[index].ckLocal();
  assert(w);
  w->setSourceTets(inpoel, coords, u);
}

void
Controller::separateCollisions(
    std::unordered_map<MeshData, std::vector<DetailedCollision>*>& outgoing,
    bool dest, int nColl, Collision* colls) {
  for (const auto& itr : proxyMap) {
    if (itr.second.dest == dest) {
      outgoing[itr.second] = new std::vector<DetailedCollision>[itr.second.m_nchare];
    }
  }

  // Separate collisions based on the mesh chare they belong to
  for (int i = 0; i < nColl; i++) {
    bool found = false;
    for (auto& itr : outgoing) {
      const auto& mesh = itr.first;
      DetailedCollision coll;
      const int aidx = colls[i].A.chunk - mesh.m_firstchunk;
      const int bidx = colls[i].B.chunk - mesh.m_firstchunk;
      if (aidx >= 0 && aidx < mesh.m_nchare) {
        if (found) CkAbort("Multiple meshes of the same type in collision\n");
        if (dest) {
          coll.dest_chunk = colls[i].A.chunk;
          coll.dest_index = colls[i].A.number;
          coll.source_chunk = colls[i].B.chunk;
          coll.source_index = colls[i].B.number;
        } else {
          coll.dest_chunk = colls[i].B.chunk;
          coll.dest_index = colls[i].B.number;
          coll.source_chunk = colls[i].A.chunk;
          coll.source_index = colls[i].A.number;
        }
        itr.second[aidx].push_back(coll);
        found = true;
      }

      if (bidx >= 0 && bidx < mesh.m_nchare) {
        if (found) CkAbort("Multiple meshes of the same type in collision\n");
        if (dest) {
          coll.dest_chunk = colls[i].B.chunk;
          coll.dest_index = colls[i].B.number;
          coll.source_chunk = colls[i].A.chunk;
          coll.source_index = colls[i].A.number;
        } else {
          coll.dest_chunk = colls[i].A.chunk;
          coll.dest_index = colls[i].A.number;
          coll.source_chunk = colls[i].B.chunk;
          coll.source_index = colls[i].B.number;
        }
        itr.second[bidx].push_back(coll);
        found = true;
      }
    }
    if (!found) CkAbort("Invalid collision in list\n");
  }
}

void
Controller::separateCollisions(
    std::unordered_map<MeshData, std::vector<DetailedCollision>*>& outgoing,
    bool dest, int nColl, DetailedCollision* colls) {
  for (const auto& itr : proxyMap) {
    if (itr.second.dest == dest) {
      // TODO: This should be made a vector probably to avoid memory management
      outgoing[itr.second] = new std::vector<DetailedCollision>[itr.second.m_nchare];
    }
  }

  // Separate collisions based on the destination mesh chare they belong to
  for (int i = 0; i < nColl; i++) {
    for (auto& itr : outgoing) {
      const auto& mesh = itr.first;
      int chunk;
      if (dest) chunk = colls[i].dest_chunk - mesh.m_firstchunk;
      else chunk = colls[i].source_chunk - mesh.m_firstchunk;
      if (chunk >= 0 && chunk < mesh.m_nchare) {
        itr.second[chunk].push_back(colls[i]);
      }
    }
  }
}

void
Controller::distributeCollisions(int nColl, Collision* colls)
// *****************************************************************************
//  Called when all potential collisions have been found, and now need to be
//  destributed to the chares in the destination mesh to determine actual
//  collisions.
//! \param[in] nColl Number of potential collisions found
//! \param[in] colls The list of potential collisions
// *****************************************************************************
{
  CkPrintf("Collisions found: %i\n", nColl);

  std::unordered_map<MeshData, std::vector<DetailedCollision>*> outgoing;
  separateCollisions(outgoing, true, nColl, colls);

  // Send out each list to the destination chares for further processing
  for (auto& itr : outgoing) {
    auto& mesh = itr.first;
    // TODO: Don't send out empty messages
    for (int i = 0; i < mesh.m_nchare; i++) {
      mesh.m_proxy[i].processCollisions(
          static_cast<int>(itr.second[i].size()),
          itr.second[i].data() );
    }
    delete[] itr.second;
  }
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

} // exam2m::

#include "NoWarning/controller.def.h"
