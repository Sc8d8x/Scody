#include <SFML/Network.hpp>
#include <string>
#include <iostream>
#include <Windows.h>
#include <thread>
#include <atomic>

class ChatApplication {
private:
    sf::TcpSocket socket;
    std::string name;
    std::atomic<bool> running;

    void setupConnection() {
        char connectionType;
        std::cout << "Enter connection type: 's' for server, 'c' for client: ";
        std::cin >> connectionType;
        std::cin.ignore(); 

        if (connectionType == 's') {
            setupServer();
        }
        else if (connectionType == 'c') {
            setupClient();
        }
        else {
            std::cerr << "Invalid connection type!" << std::endl;
            return;
        }

        std::cout << "Enter your name: ";
        std::getline(std::cin, name);
    }

    void setupServer() {
        sf::TcpListener listener;
        if (listener.listen(2000) != sf::Socket::Done) {
            std::cerr << "Failed to bind port 2000!" << std::endl;
            return;
        }

        std::cout << "Server started. Waiting for client connection..." << std::endl;

        if (listener.accept(socket) != sf::Socket::Done) {
            std::cerr << "Failed to accept client connection!" << std::endl;
            return;
        }

        std::cout << "Client connected!" << std::endl;
    }

    void setupClient() {
        std::string ipAddress;
        std::cout << "Enter server IP address (localhost for same machine): ";
        std::getline(std::cin, ipAddress);

        sf::IpAddress ip = (ipAddress == "localhost") ? sf::IpAddress::getLocalAddress() : sf::IpAddress(ipAddress);

        std::cout << "Connecting to server..." << std::endl;

        if (socket.connect(ip, 2000) != sf::Socket::Done) {
            std::cerr << "Failed to connect to server!" << std::endl;
            return;
        }

        std::cout << "Connected to server!" << std::endl;
    }

    void receiveMessages() {
        sf::Packet packet;
        while (running) {
            if (socket.receive(packet) == sf::Socket::Done) {
                std::string senderName, message;
                if (packet >> senderName >> message) {
                    std::cout << "\n" << senderName << ": " << message << std::endl;
                    std::cout << "> " << std::flush;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void sendMessages() {
        std::string message;
        while (running) {
            std::cout << "> ";
            std::getline(std::cin, message);

            if (!message.empty()) {
                if (message == "/exit") {
                    running = false;
                    break;
                }

                sf::Packet packet;
                packet << name << message;
                if (socket.send(packet) != sf::Socket::Done) {
                    std::cerr << "Failed to send message!" << std::endl;
                }
            }
        }
    }

public:
    ChatApplication() : running(false) {}

    void run() {
        setupConnection();
        if (socket.getRemoteAddress() == sf::IpAddress::None)
        {
            std::cerr << "Connection failed. Exiting..." << std::endl;
            return;
        }

        running = true;
        socket.setBlocking(false);

        std::thread receiverThread(&ChatApplication::receiveMessages, this);
        sendMessages();

        running = false;
        if (receiverThread.joinable()) {
            receiverThread.join();
        }
        socket.disconnect();
    }
};

int main() {
    std::cout << "Console Chat Application" << std::endl;
    std::cout << "------------------------" << std::endl;

    char choice;
    std::cout << "Start application? (s - start, any other key - exit): ";
    std::cin >> choice;
    std::cin.ignore(); 

    if (tolower(choice) == 's') {
        ChatApplication app;
        app.run();
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
