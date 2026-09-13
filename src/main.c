#include <stdlib.h>
#include <time.h>

#include "application.h"

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    return run_application(argc, argv);
}
