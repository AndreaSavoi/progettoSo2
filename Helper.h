#include <unistd.h>


ssize_t safeRead(int fd, void *buff, size_t maxlen);
ssize_t safeWrite(int fc, const void *buff, size_t maxlen);
void handle_error_rs(ssize_t ret_valu, int sockfd);
void handle_error_ws(ssize_t ret_value, int sockfd);
void handle_error_rc(ssize_t ret_valu, int sockfd);
void handle_error_wc(ssize_t ret_value, int sockfd);