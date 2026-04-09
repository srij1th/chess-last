#include "server.h"

#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned int)time(NULL));
    return run_http_server(8080);
}
