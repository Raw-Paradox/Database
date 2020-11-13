#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include "bplus.h"

int main(int argc, char const* argv[]) {
    table_t* table = open_db("/Users/chenyibo/Code/database/server/test.db");
    srand(time(NULL));
    // for (size_t i = 0; i < 15; i++) {
    //     int random = rand() % 15 + i * 15;
    //     key_t key = {random};
    //     row_t row = {random, "hello world", "hello@world.tju"};
    //     insert(table, key, row);
    // }
    print_table(table);
    close_db(table);
    return 0;
}
