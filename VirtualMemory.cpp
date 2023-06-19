//
// Created by ilaysoffer on 6/19/23.
//

#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "math.h"

/*
 * Initialize the virtual memory.
 */
void VMinitialize() {
    for ( int i = 0; i < pow(2, OFFSET_WIDTH); i++) {
        PMwrite(0 + i, 0);
    }
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
    uint64_t virtual_address_no_offset = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;
    uint64_t size_of_mini_address = virtual_address_no_offset / TABLES_DEPTH;


    word_t address_first = 0;
    word_t address_next;
    word_t finish_address;

    uint64_t framesNotEvict[TABLES_DEPTH + 1];

    for (int i = 0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = address_first;
        uint64_t current_mini_word =
                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * i))
                & ~(~0 << (size_of_mini_address + 1))); //msb
        PMread(address_first + current_mini_word, &address_next);
        if (address_next == 0) {
            address_first = find_empty_frame();
            if (address_first != 0)
                continue;
            address_first = find_unused_frame();
            if (address_first != 0)
                continue;
            address_first = find_min_cyclic_distance();
            if (address_first != 0)
                continue;
        }

    }
    uint64_t offset_mini_word =
            (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * TABLES_DEPTH))
             & ~(~0 << (size_of_mini_address + 1))); //msb
    PMread(finish_address + offset_mini_word, value);
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value);

