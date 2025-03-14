#include <stdio.h>
#include <ctype.h>
#include "cmd_parser.h"
#include "scheduling.h"

int main(void) {
    start_scheduler();
    cmd_parser();
}
