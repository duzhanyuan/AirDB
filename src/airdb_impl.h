#ifndef AIRDB_NODE_IMPL_H_
#define AIRDB_NODE_IMPL_H_

#include "proto/airdb_node.pb.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <stdint.h>
#include <unistd.h>
#include "common/mutex.h"
#include "common/thread_pool.h"
#include "rpc_client.h"
#include "binlog.h"
#include "log.h"
#include "logger.h"
#include "config.h"
#include "common/mutex.h"
#include "common/macro.h"
#include "storage_kv.h"

namespace airdb {
 
using common::ThreadPool;
using common::Mutex;
using common::MutexLock;
using common::CondVar;

class BinLogger;

struct SdkAck {
    PutResponse* put_res;
    google::protobuf::Closure* done;
    SdkAck() : put_res(NULL), done(NULL){} 

};


class AirDBImpl : public AirDB {
public:
    AirDBImpl(std::string& server_id, const std::vector<std::string>& members);
    virtual ~AirDBImpl();
public:    
    void LoopVoteLeader();
    int32_t GetRandTimeOut();
    void VoteLeader();
    void GetLocalLogTermIndex(int64_t* last_log_index, 
    				int64_t* last_log_term);
    void VoteCallBack(const VoteRequest* req, 
    		      VoteResponse* res, bool failed, int error);
    void SwitchToLeader();
    void SwitchToFollower(int64_t term);
    void BroadCastHeartBeat();
    void StartReplicaLog();
    void ReplicateLog(const std::string& follower_addr);
    void UpdateCommitIndex(int64_t index);
    void CommitIndex(); 
    void ParseValue(const std::string& value, BinLogOperation& op, std::string& real_value);
    void AppendEntries(::google::protobuf::RpcController*, 
    		       const AppendEntriesRequest* req, 
		       AppendEntriesResponse* res, 
		       ::google::protobuf::Closure* done);
    void AppendEntriesImpl(const AppendEntriesRequest* req,
			   AppendEntriesResponse* res,
			   ::google::protobuf::Closure* done);
    void ShowStatus(::google::protobuf::RpcController* rpc,
    		    const ShowStatusRequest* req,
    		    ShowStatusResponse* res,
		    ::google::protobuf::Closure* done);
    void HeartBeatCallBack(const AppendEntriesRequest* req,
    			   AppendEntriesResponse* res,
			   bool filed, int error);
    void Vote(google::protobuf::RpcController* controller, 
    		const VoteRequest* req, VoteResponse* res, 
		google::protobuf::Closure* done);
    void Put(google::protobuf::RpcController* controller,
		const PutRequest* req, PutResponse* res,
		google::protobuf::Closure* done);
    void Get(google::protobuf::RpcController* controller,
		const GetRequest* req, GetResponse* res,
		google::protobuf::Closure* done);
public:
    RpcClient rpc_client_;
    BinLogger* binlogger_;
    ThreadPool loop_vote_pool_;
    ThreadPool heart_beat_pool_;
    ThreadPool follower_work_pool_;
    ThreadPool replica_pool_;
    ThreadPool commit_pool_;
    Mutex mu_;
    CondVar* replica_cond_;
    CondVar* commit_cond_;
    bool stop_;
    DBNodeStatus status_;
    int64_t headbeat_cnt_;
    int64_t current_term_;
    std::string self_addr_;
    std::string cur_leader_addr_;
    int64_t commit_index_;
    int64_t last_applied_index_;
    std::vector<std::string> all_server_addr_;
    std::map<int64_t, std::string> vote_for_;
    std::map<int64_t, int> vote_pass_;
    std::map<std::string, int64_t> next_index_;
    std::map<std::string, int64_t> match_index_;
    std::set<std::string> replicating_;
    boost::unordered_map<int64_t, SdkAck> sdk_ack_;
    StorageKV* data_store_;
};


}//end namespace



#endif

