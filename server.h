#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


using namespace std;

class Message {
public:
  Message(const string& topic, const string& data) : topic_(topic), data_(data) {}

  const string& GetTopic() const { return topic_; }
  const string& GetData() const { return data_; }

private:
  string topic_;
  string data_;
};

class PubSubServer {
public:
  PubSubServer();
  
  void Start() ;
  void Publish(const Message& message);
  Message GetNextMessage();

private:
  std::mutex mutex_;
  std::condition_variable condition_variable_;
  queue<Message> queue_;
};