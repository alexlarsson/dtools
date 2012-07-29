typedef enum {
  FORMAT_TEXT,
  FORMAT_BINARY
} StreamFormat;

#define BINARY_FORMAT_MAGIC_COOKIE 0x25458dea

void pipe_setup_for_binary (int fd);
StreamFormat pipe_negotiate_format (int fd);
