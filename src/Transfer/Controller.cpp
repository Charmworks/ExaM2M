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

//! \brief Global readonly to access the controller group
/* readonly */ CProxy_Controller controllerProxy;
//! \brief Charm handle to the collision detection library instance
/* readonly */ CollideHandle collideHandle;

//!\brief Function called by charm collision library when it completes
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
      CollideDistributedClient(CkCallback(exam2m::CkIndex_Controller::distributeCollisions(NULL),controllerProxy)));
      //CollideSerialClient(collisionHandler, 0));
}

Controller::Controller() : current_chunk(0) {}

void
Controller::addMesh(CkArrayID p, int elem, CkCallback cb)
//! \brief Creates a worker array bound to the passed in mesh array
{
  auto id = static_cast<std::size_t>(CkGroupID(p).idx);
  if (proxyMap.count(id) == 0) {
    // Create CkArrayOptions to bind to the user array with correct num elems
    CkArrayOptions opts;
    opts.bindTo(p);
    opts.setNumInitial(elem);

    // Create mesh data, library-side worker array, and add it to the map
    MeshData mesh;
    mesh.m_nchare = elem;
    mesh.m_firstchunk = current_chunk;
    mesh.m_proxy = CProxy_Worker::ckNew(p, mesh, cb, opts);
    proxyMap[id] = mesh;

    // Update number of total chunks
    current_chunk += elem;
  } else {
    CkAbort("ERROR: Trying to add the same mesh mutliple times\n");
  }
}

void
Controller::setMesh( CkArrayID p, MeshData d )
//! \brief Called from Worker ctor to ensure mesh data is set on all PEs
{
  proxyMap[static_cast<std::size_t>(CkGroupID(p).idx)] = d;
}

void
Controller::setDestPoints(CkArrayID p, int index, tk::UnsMesh::Coords* coords,
    const tk::Fields& u, CkCallback cb)
//! \brief Sets the designated mesh as a destination mesh and passes pointers
//         to the destination mesh data.
{
  proxyMap[CkGroupID(p).idx].dest = true;
  Worker* w = proxyMap[CkGroupID(p).idx].m_proxy[index].ckLocal();
  assert(w);
  w->setDestPoints(coords, u, cb);
}

void
Controller::setSourceTets(CkArrayID p, int index,
    std::vector< std::size_t >* inpoel, tk::UnsMesh::Coords* coords,
    const tk::Fields& u)
//! \brief Sets the designated mesh as a source mesh and passes pointers to the
//         source mesh data.
{
  proxyMap[CkGroupID(p).idx].dest = false;
  Worker* w = proxyMap[CkGroupID(p).idx].m_proxy[index].ckLocal();
  assert(w);
  w->setSourceTets(inpoel, coords, u);
}

void
Controller::separateCollisions(MeshDict& outgoing, bool dest, int nColl,
                               Collision* colls) const
// *****************************************************************************
//  Called on a list of potential collisions in order to separate them based on
//  the chare they are intended for, and to convert them into the more useful
//  DetailedCollision struct.
//! \param[inout] outgoing The map of lists where we will divide up collisions
//! \param[in] dest  True if we want to separate for dest mesh, false for source
//! \param[in] nColl Number of collisions we are separating
//! \param[in] colls The list of collisions to separate
// *****************************************************************************
{
  for (const auto& itr : proxyMap) {
    if (itr.second.dest == dest) {
      outgoing[itr.second].resize(itr.second.m_nchare);
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
Controller::separateCollisions(MeshDict& outgoing, bool dest, int nColl,
                               DetailedCollision* colls) const
// *****************************************************************************
//  Called on a list of potential collisions in order to separate them based on
//  the chare they are intended for. This version of the function is for
//  collisions which have already been converted to DetailedCollision.
//! \param[inout] outgoing The map of lists where we will divide up collisions
//! \param[in] dest  True if we want to separate for dest mesh, false for source
//! \param[in] nColl Number of collisions we are separating
//! \param[in] colls The list of collisions to separate
// *****************************************************************************
{
  for (const auto& itr : proxyMap) {
    if (itr.second.dest == dest) {
      outgoing[itr.second].resize(itr.second.m_nchare);
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
  CkPrintf("[%i]: Collisions found: %i\n", CkMyPe(), nColl);

  MeshDict outgoing;
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
  }
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

} // exam2m::

#include "NoWarning/controller.def.h"
