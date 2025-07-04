#include<stdio.h>
#include "./lib/examples/ntdll/memory.h"
#include "./lib/examples/win32u/clipboard.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: VoidCalls.exe <EXAMPLE_INDEX>\n");
        return -1;
    }

    int choice = atoi(argv[1]);

    switch (choice) {
    case 0:
        run_memory_example();
        break;
    case 1:
        run_clipboard_example();
        break;

    default:
        printf("[-] Invalid example\n");
        return -1;
    }

    return 0;
}