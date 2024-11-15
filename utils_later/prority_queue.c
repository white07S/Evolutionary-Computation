// main.c
#include <stdio.h>

// Function prototypes (from assembly)
void insert(long value);
long extract_max();
void print_int(long value);

int main() {
    // Insert values into the priority queue
    insert(0x5);    // 5 in hex
    insert(0xA);    // 10 in hex
    insert(0x3);    // 3 in hex
    insert(0x8);    // 8 in hex
    insert(0x2);    // 2 in hex

    // Attempt to insert more values to test overflow
    for(long i = 0; i < 100; i++) {
        insert(i);
    }

    // Extract the maximum value from the priority queue
    long max = extract_max();

    // Print the extracted maximum value
    print_int(max);

    return 0;
}
