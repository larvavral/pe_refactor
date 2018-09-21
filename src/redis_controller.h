#ifndef REDIS_CONTROLLER_H_
#define REDIS_CONTROLLER_H_

#include <functional>
#include <string>
#include "hiredis/async.h"
#include "hiredis/hiredis.h"

// Use to work with hiredis.
namespace redis {

namespace client {

// Create new redis client (synchronous connection).
redisContext* CreateRedisClient(const std::string& host, int port);

// Perform 'AUTH' command.
bool Authenticate(redisContext* redis_context, const std::string& password);

// Perform 'GET' command.
std::string Get(redisContext* redis_context, const std::string& key);

// Perform 'SET' command.
void Set(redisContext* redis_context,
         const std::string& key,
         const std::string& value);

} // namespace client

namespace async_connect {

typedef std::function<void(redisReply*)> AsyncCommandCallback;

// Create new asynchronous connection to redis server.
redisAsyncContext* CreateAsyncConnect(const std::string& host, int port);

// Perform authenticate command on async connection.
void Authenticate(redisAsyncContext* async_connect,
                  const std::string& password,
                  AsyncCommandCallback callback);

// Subcribe a specific channel on redis server to listening message.
void Subscribe(redisAsyncContext* async_connect,
               const std::string& channel,
               AsyncCommandCallback callback);

void Publish(redisAsyncContext* async_connect,
             const std::string& channel,
             const std::string& message,
             AsyncCommandCallback callback);

} // namespace asyn_connect

} // namespace redis

#endif  // REDIS_CONTROLLER_H_
