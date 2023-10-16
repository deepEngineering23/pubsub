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

  void Connect() {
    // Create a TCP socket.
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
      cerr << "Failed to create client socket" << endl;
      exit(1);
    }

    // Resolve the server address.
    struct hostent *server_hostent = gethostbyname(server_address_.c_str());
    if (server_hostent == NULL) {
      cerr << "Failed to resolve server address" << endl;
      exit(1);
    }

    // Connect to the server.
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port_);
    server_address.sin_addr = *(in_addr*)server_hostent->h_addr;

    if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
      cerr << "Failed to connect to server" << endl;
      exit(1);
    }

    // Start a thread to receive messages from the server.
    thread message_receiver_thread([this, client_socket]() {
      while (true) {
        // Receive a message from the server.
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
          break;
        }

        // Parse the message.
        string topic(buffer);
        string data(buffer + topic.length() + 1);

        // Notify the client that a new message has been received.
        OnMessageReceived(topic, data);
      }

      // Close the client socket.
      close(client_socket);
    });

    message_receiver_thread.detach();
  }

  void Subscribe(const string& topic) {
    // Send a subscription request to the server.
    string message = "SUBSCRIBE " + topic;
    send(client_socket_, message.c_str(), message.length(), 0);
  }

  void Publish(const string& topic, const string& data) {
    // Send a publish request to the server.
    string message = "PUBLISH " + topic + " " + data;
    send(client_socket_, message.c_str(), message.length(), 0);
  }

  virtual void OnMessageReceived(const string& topic, const string& data) = 0;

private:
  string server_address_;
  int server_port_;
  int client_socket_;
};