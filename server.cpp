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
  PubSubServer() {}

  void Start() {
    // Create a TCP socket.
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
      cerr << "Failed to create server socket" << endl;
      exit(1);
    }

    // Bind the socket to the port.
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(5000);

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
      cerr << "Failed to bind server socket to port" << endl;
      exit(1);
    }

    // Listen for incoming connections.
    if (listen(server_socket, 10) < 0) {
      cerr << "Failed to listen for incoming connections" << endl;
      exit(1);
    }

    // Start a thread to handle incoming connections.
    thread connection_handler_thread([this, server_socket]() {
      while (true) {
        // Accept an incoming connection.
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
          cerr << "Failed to accept incoming connection" << endl;
          continue;
        }

        // Create a new thread to handle the client connection.
        thread client_handler_thread([this, client_socket]() {
          // Handle the client connection.
          while (true) {
            // Receive a message from the client.
            char buffer[1024];
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
              break;
            }

            // Parse the message.
            string topic(buffer);
            string data(buffer + topic.length() + 1);

            // Create a new message object and publish it to the topic.
            Message message(topic, data);
            Publish(message);
          }

          // Close the client socket.
          close(client_socket);
        });

        client_handler_thread.detach();
      }
    });

    connection_handler_thread.detach();
  }

  void Publish(const Message& message) {
    // Lock the mutex.
    std::unique_lock<std::mutex> lk(mutex_);

    // Add the message to the queue.
    queue_.push(message);

    // Signal the condition variable.
    condition_variable_.notify_one();

    // Unlock the mutex.
    lk.unlock();
  }

  Message GetNextMessage() {
    // Lock the mutex.
    std::unique_lock<std::mutex> lk(mutex_);

    // Wait for a message to be available.
    condition_variable_.wait(lk, [&]() { return !queue_.empty(); });

    // Get the next message from the queue.
    Message message = queue_.front();
    queue_.pop();

    // Unlock the mutex.
    lk.unlock();

    return message;
  }

private:
  std::mutex mutex_;
  std::condition_variable condition_variable_;
  queue<Message> queue_;
};