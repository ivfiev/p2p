#ifndef P2P_LOG_H
#define P2P_LOG_H

#define ERROR_FATAL(msg) err_fatal("%s at %s:%d", msg, __FILE_NAME__, __LINE__)
#define ERROR_INFO(msg) err_info("%s at %s:%d", msg, __FILE_NAME__, __LINE__)

struct log_data {
  int stdout_fd;
  int stderr_fd;
  int socket_fd;
  char *port;
  bool reconnect;
};

void err_fatal(const char *format, ...) __attribute__((format(printf, 1, 2)));

void err_info(const char *format, ...) __attribute__((format(printf, 1, 2)));

void log_debug(const char *format, ...) __attribute__((format(printf, 1, 2)));

void log_info(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif //P2P_LOG_H
