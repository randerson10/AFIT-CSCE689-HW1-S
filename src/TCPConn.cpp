#include "TCPConn.h"


TCPConn::TCPConn() {
}

TCPConn::~TCPConn() {

}

bool TCPConn::accept(SocketFD &server) {
    return false;
}
int TCPConn::sendText(const char *msg) {
    return 0;
}
int TCPConn::sendText(const char *msg, int size) {
    return 0;
}
void TCPConn::handleConnection() {

}
void TCPConn::startAuthentication() {

}
void TCPConn::getUsername() {

}
void TCPConn::getPasswd() {

}
void TCPConn::sendMenu() {

}
void TCPConn::getMenuChoice() {

}
void TCPConn::setPassword() {

}
void TCPConn::changePassword() {

}
bool TCPConn::getUserInput(std::string &cmd) {
    return false;
}
void TCPConn::disconnect() {

}
bool TCPConn::isConnected() {
    return false;
}
