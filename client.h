#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

class PubSubClient {
public:
  PubSubClient(const string& server_address, const int server_port) : server_address_(server_address), server_port_(server_port) {}

  void Connect();
  void Subscribe(const string& topic);
  void Publish(const string& topic, const string& data);
  virtual void OnMessageReceived(const string& topic, const string& data) = 0;

private:
  string server_address_;
  int server_port_;
  int client_socket_;
};