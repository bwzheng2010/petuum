// author: jinliang

#pragma once

#include <vector>
#include <pthread.h>
#include <boost/unordered_map.hpp>
#include <string>
#include "petuum_ps/include/table.hpp"
#include "petuum_ps/include/configs.hpp"
#include "petuum_ps/include/abstract_row.hpp"
#include "petuum_ps/client/row_metadata.hpp"
#include "petuum_ps/util/vector_clock.hpp"
#include "petuum_ps/server/server_table.hpp"

namespace petuum {
struct ServerRowRequest {
public:
  int32_t bg_id; // requesting bg thread id
  int32_t table_id;
  int32_t row_id;
  int32_t clock;
};

// 1. Manage the table storage on server;
// 2. Manage the pending reads;
// 3. Manage the vector clock for clients
// 4. (TODO): manage OpLogs that need acknowledgements

class Server {
public:
  Server();
  ~Server();

  void AddClientBgPair(int32_t client_id, int32_t bg_id);
  void Init();
  void Init(const std::string &snapshot_dir, int32_t snapshot_clock,
    int32_t server_id, int32_t resume_clock);

  void CreateTable(int32_t table_id, TableInfo &table_info);
  ServerRow *FindCreateRow(int32_t table_id, int32_t row_id);
  bool Clock(int32_t client_id, int32_t bg_id);
  void AddRowRequest(int32_t bg_id, int32_t table_id, int32_t row_id,
    int32_t clock);
  void GetFulfilledRowRequests(std::vector<ServerRowRequest> *requests);
  void ApplyOpLog(const void *oplog, int32_t bg_thread_id,
    uint32_t version);
  int32_t GetMinClock();
  int32_t GetBgVersion(int32_t bg_thread_id);
private:
  VectorClock client_clocks_;
  std::map<int32_t, VectorClock> client_vector_clock_map_;
  std::map<int32_t, std::vector<int32_t> > client_bg_map_;

  boost::unordered_map<int32_t, ServerTable> tables_;
  // mapping <clock, table id> to an array of read requests
  std::map<int32_t,
    boost::unordered_map<int32_t,
      std::vector<ServerRowRequest> > > clock_bg_row_requests_;
  std::vector<int32_t> client_ids_;
  // latest oplog version that I have received from a bg thread
  std::map<int32_t, uint32_t> bg_version_map_;

  int32_t snapshot_clock_;
  std::string snapshot_dir_;
  int32_t server_id_;
  int32_t resume_clock_;
};

}  // namespace petuum
