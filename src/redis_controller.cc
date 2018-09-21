#include "redis_controller.h"

#include "glog/logging.h"

namespace redis {

namespace client {

redisContext* CreateRedisClient(const std::string& host, int port) {
  redisContext* redis_context = redisConnect(host.c_str(), port);
  if (redis_context == nullptr || redis_context->err) {
    LOG(ERROR) << "Cannot create redis client.";
    return nullptr;
  }

  LOG(INFO) << "Create new redis client successfully.";
  return redis_context;
}

bool Authenticate(redisContext* redis_context, const std::string& password) {
  bool result = false;
  redisReply* reply =
      (redisReply*) redisCommand(redis_context, "AUTH %s", password.c_str());
  if (reply->type == REDIS_REPLY_ERROR) {
    LOG(ERROR) << "Authenticate failed!";
    result = false;
  } else {
    LOG(INFO) << "Authenticate successfully.";
    result = true;
  }

  freeReplyObject(reply);
  return result;
}

std::string Get(redisContext* redis_context, const std::string& key) {
  std::string result;
  redisReply* reply =
      (redisReply*) redisCommand(redis_context, "GET %s", key.c_str());
  if (reply->type == REDIS_REPLY_ERROR ||
      reply->type == REDIS_REPLY_NIL) {
    LOG(ERROR) << "Cannot get value of key = " << key;
  } else if (reply->type == REDIS_REPLY_STRING) {
    result = std::string(reply->str);
  }

  freeReplyObject(reply);
  return result;
}

void Set(redisContext* redis_context,
         const std::string& key,
         const std::string& value) {
  redisReply* reply =
      (redisReply*) redisCommand(redis_context, "SET %s %s", key.c_str(), value.c_str());
  freeReplyObject(reply);
}

} // namespace client

namespace async_connect {

// Helper class to excute async command with a non-static callback.
template<typename Callback>
class Handler {
public:
  Handler(Callback cb) : cb_(cb) {}

  static void callback(redisAsyncContext *c, void *reply, void *privdata) {
    (static_cast<Handler<Callback>*>(privdata))->operator()(c, reply);
  }

  void operator()(redisAsyncContext *context, void *reply) {
      if (reply && cb_) {
          cb_(static_cast<redisReply*>(reply));
      }

      // TODO(hoangpq): Find out mechanism to delete Handler pointer.
      // Maybe seperate subscribe command and other commands.
      // delete(this);
  }

private:
  Callback cb_;
};

redisAsyncContext* CreateAsyncConnect(const std::string& host, int port) {
  redisAsyncContext* async_connect = redisAsyncConnect(host.c_str(), port);
  if (async_connect == nullptr || async_connect->err) {
    LOG(ERROR) << "Create async connection to redis server failed!";
    return nullptr;
  }

  LOG(INFO) << "Create async connection to redis server successfully.";
  return async_connect;
}

void Authenticate(redisAsyncContext* async_connect,
                  const std::string& password,
                  AsyncCommandCallback callback) {
  Handler<AsyncCommandCallback> *handler =
      new Handler<AsyncCommandCallback>(callback);
  redisAsyncCommand(async_connect,
                    Handler<AsyncCommandCallback>::callback,
                    handler,
                    "AUTH %s",
                    password.c_str());
}

void Subscribe(redisAsyncContext* async_connect,
               const std::string& channel,
               AsyncCommandCallback callback) {
  Handler<AsyncCommandCallback> *handler =
      new Handler<AsyncCommandCallback>(callback);
  redisAsyncCommand(async_connect,
                    Handler<AsyncCommandCallback>::callback,
                    handler,
                    "SUBSCRIBE %s",
                    channel.c_str());
}

void Publish(redisAsyncContext* async_connect,
             const std::string& channel,
             const std::string& message,
             AsyncCommandCallback callback) {
  Handler<AsyncCommandCallback> *handler =
      new Handler<AsyncCommandCallback>(callback);
  redisAsyncCommand(async_connect,
                    Handler<AsyncCommandCallback>::callback,
                    handler,
                    "PUBLISH %s %s",
                    channel.c_str(),
                    message.c_str());
}

} // namespace async_connect

} // namespace redis