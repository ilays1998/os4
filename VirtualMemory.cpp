//
// Created by ilaysoffer on 6/19/23.
//

#include "VirtualMemory.h"
#include "PhysicalMemory.h"


/*
 * Initialize the virtual memory.
 */
void VMinitialize() {
    for ( int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMwrite(0 + i, 0);
    }
}
int abs(int a, int b) {
    if (a > b)
        return a - b;
    return b - a;
}
int cyclic_distance(int virtualAddress, int currentPage) {
    int min = 0;
    int absolute_distance  = abs(virtualAddress, currentPage);
    min = NUM_PAGES - absolute_distance;
    if (min < absolute_distance) {
        min = absolute_distance;
    }
    return min;
}

//uint64_t bitwiseAddTheNewFrameAddressBit(uint64_t pageByBit, word_t frame_address, int depth) {
//    uint64_t raleventBits = (frame_address >> (PHYSICAL_ADDRESS_WIDTH - (size_of_mini_address * TABLES_DEPTH))
//     & ~(~0 << (size_of_mini_address + 1)));
//    return pageByBit | raleventBits;
//}
word_t find_empty_frame(word_t frame_address, int depth, int *max_frame_index, int *min_cyclic_distance
                        , uint64_t virtualAddressWithoutOffset, uint64_t pageByBit) {
    word_t tmp;
    word_t return_frame = -1;
    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame_address + i, &tmp);
        if (tmp != 0)
            break;
    }
    if (tmp == 0 && frame_address != 0) { //TODO: if frame return 0
        return frame_address;
    }
    if (depth >= TABLES_DEPTH) {
        return -1;
    }//TODO: check edge case

    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame_address + i, &tmp);
        if (*max_frame_index < tmp) {
            *max_frame_index = tmp;
        }
        if (tmp != 0) {
            return_frame = find_empty_frame(tmp, depth++, max_frame_index, min_cyclic_distance,
                                            virtualAddressWithoutOffset,
                                            bitwiseAddTheNewFrameAddressBit(pageByBit, i, depth) );
        }
        if (return_frame != -1) {
            return return_frame;
        }
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
    int max_frame_index = 0;
    int min_cyclic_distance = NUM_PAGES;
    uint64_t framesNotEvict[TABLES_DEPTH + 1];

    for (int i = 0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = address_first;
        uint64_t current_mini_word =
                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * i))
                & ~(~0 << (size_of_mini_address + 1))); //msb
        PMread((address_first  * PAGE_SIZE) + current_mini_word, &address_next);
        if (address_next == 0) {
            address_first = find_empty_frame(address_first, i,
                                             &max_frame_index, &min_cyclic_distance,
                                             current_mini_word, 0);
            if (address_first != -1)
                continue;
            if (max_frame_index + 1 < NUM_FRAMES) {
                address_first = max_frame_index + 1;
                continue;
            }
            address_first = min_cyclic_distance;
            //swap out the frame
        }

    }
    uint64_t offset_mini_word =
            (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * TABLES_DEPTH))
             & ~(~0 << (size_of_mini_address + 1))); //msb
    PMread((finish_address * PAGE_SIZE) + offset_mini_word, value);
}



/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value) {

    //PMwrite((finish_address * PAGE_SIZE) + offset_mini_word, value);
}

