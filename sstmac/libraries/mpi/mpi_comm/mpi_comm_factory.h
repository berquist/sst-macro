/*
 *  This file is part of SST/macroscale:
 *               The macroscale architecture simulator from the SST suite.
 *  Copyright (c) 2009 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the LICENSE file in the top
 *  SST/macroscale directory.
 */

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_MPICOMMFACTORY_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_MPICOMMFACTORY_H_INCLUDED

#include <sstmac/libraries/mpi/mpi_comm/mpi_comm.h>
#include <sstmac/libraries/mpi/mpi_comm/mpi_comm_id.h>
#include <sstmac/libraries/mpi/mpi_types/mpi_id.h>
#include <sstmac/libraries/mpi/mpi_types/mpi_type.h>
#include <sstmac/libraries/mpi/mpi_api_fwd.h>
#include <sstmac/software/process/task_id.h>
#include <sstmac/software/process/app_id.h>

namespace sstmac {
namespace sw {


/**
 * Construct mpi communicators.
 */
class mpi_comm_factory  {
  /// The private type used to manage communicators under construction.
  class pending;
  class pending_create;
  class pending_split;

 public:
  virtual std::string
  to_string() const {
    return "mpicommfactory";
  }

  /// Build comm_world using information retrieved from the environment.
  mpi_comm_factory(app_id aid, mpi_api* parent);

  /// Goodbye.
  virtual ~mpi_comm_factory();

  /// Initialize the object.
  void init(app_manager* env, mpi_id rank);

  void finalize();

 public:
  /// Get the world communicator for the given node.
  /// In this communicator, mpiid indices are the same as the taskid
  /// indices given by the environment.  Each node will only have one world.
  mpi_comm*
  world(){
    return worldcomm_;
  }

  /// Get a 'self' communicator.  Each node will have a unique index
  /// for its self.  Each node will only have one self.
  mpi_comm*
  self(){
    return selfcomm_;
  }


  /// Duplicate the given communicator.
  /// Blocks unti lall nodes have entered the call.
  /// Eventually we may opt to deal with the extended attributes (keyval)
  /// stuff in the MPI standard, even though I don't know anybody
  /// who actually uses it.
  mpi_comm* comm_dup(mpi_comm*caller);

  /// Make the given mpiid refer to a newly created communicator.
  /// Blocks the caller until all nodes have entered the call.
  /// This call must be performed by all nodes in the caller communicator
  /// a creating any new communicators.  Returns mpicomm::null_comm
  /// on all nodes that are not participants the new group.
  mpi_comm* comm_create(mpi_comm*caller, mpi_group* group);

  /// MPI_Comm_split -- collective operation.
  mpi_comm* comm_split(mpi_comm*caller, int color, int key);

  mpi_comm* create_cart(mpi_comm*caller, int ndims, int *dims,
                            int *periods,
                            int reorder);

 protected:

  mpi_api* parent_;

  app_id aid_;

  /// We can restrict our run to use fewer than the nodes allocated.
  int mpirun_np_;

  /// The next available communicator index.
  mpi_comm_id next_id_;

  /// Keyring(s) for pending comm_dup or comm_create requests.
  /// The signature for each pending request is determined by the
  /// input communicator (construction of communicators is global (blocking)
  /// across the communicator).
  typedef std::map<mpi_comm_id, pending*> buildmap_t;
  buildmap_t under_construction_;

  mpi_comm* worldcomm_;
  mpi_comm* selfcomm_;
  mpi_type* splittype_;
};

}
} // end of namespace sstmac.

#endif

