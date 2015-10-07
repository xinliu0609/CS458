#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char* argv[])
{
  if (argc != 4) {
    printf("./exec CertFile KeyFile port");
    return -1;
  }

  char* cert_file = argv[1];
  char* key_file = argv[2];
  int port = atoi(argv[3]);

  // init the ssl lib
  SSL_library_init();

  //SSL_METHOD* method;
  //SSL_CTX *ctx;
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  const SSL_METHOD* method = SSLv3_server_method();
  SSL_CTX *ctx = SSL_CTX_new(method);

  // load the server's certificate
  SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM);
  // load the server's private key
  SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM);
  // check the private against the known certificate
  if (!SSL_CTX_check_private_key(ctx)) {
    printf("Private key does not match\n");
    abort();
  }

  // standard tcp server setup and connection
  int sd, client;
  struct sockaddr_in addr;
  sd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sd, (struct sockaddr*)&addr, sizeof(addr));
  listen(sd, 10);
  client = accept(sd, 0, 0);

  SSL* ssl = SSL_new(ctx);
  SSL_set_fd(ssl, client);
  SSL_accept(ssl);

  char buf[1024];
  int buf_size = 1024;

  // ...now you can read/write
  while(1) {
    int bytes = SSL_read(ssl, buf, buf_size);
    if (bytes > 0) {
      // ...process request
      
      // turn this into a legitimate string
      buf[bytes] = '\0';

      int challenge = atoi(buf);
      char reply[buf_size];
      sprintf(reply, "%d", challenge + 1);
      SSL_write(ssl, reply, strlen(reply));
    } else {
      printf("Server gets out of the loop. Bye!\n");
      break;
    }
  }

  client = SSL_get_fd(ssl);
  SSL_free(ssl);
  close(sd);
}
