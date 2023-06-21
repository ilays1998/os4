#include "VirtualMemory.h"
#include "PhysicalMemory.h"


int abs(int a, int b) {
    if (a > b)
        return a - b;
    return b - a;
}

/*
 * Initialize the virtual memory.
 */
void VMinitialize() {
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
    int virtual_address_no_offset_length = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;
    int word_length = virtual_address_no_offset_length / TABLES_DEPTH;
    uint64_t offset_mini_word =
            (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (word_length * TABLES_DEPTH))
             & ~(~0 << (word_length + 1)));
    word_t address_first = 0;
    word_t address_next;
    bool address_found = true;
    for (int i = 0; i < TABLES_DEPTH - 1; i++){
        uint64_t current_mini_word =
                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (word_length * i))
                 & ~(~0 << (word_length + 1))); //msb
        PMread((address_first * PAGE_SIZE) + current_mini_word, &address_next);
        if (address_next == 0){
            address_found = false;
            break;
        }
    }
    if (address_found){
        PMread((address_next * PAGE_SIZE) + offset_mini_word, value);
        return 1;
    }
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){

}
