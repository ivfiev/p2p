#ifndef P2P_SOCKET_H
#define P2P_SOCKET_H

int listen1(const char *port);

int connect1(const char *port);

char *getname(int socket_fd);

#endif //P2P_SOCKET_H
