//
// Created by ilaysoffer on 6/19/23.
//

#include "VirtualMemory.h"
#include "PhysicalMemory.h"


/*
 * Initialize the virtual memory.
 */
void VMinitialize() {
    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
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
    if (min > absolute_distance) {
        min = absolute_distance;
    }
    return min;
}

uint64_t bitwiseAddTheNewFrameAddressBit(uint64_t pageByBit, word_t frame_address, int depth) {
    uint64_t raleventBits = (frame_address << (depth* OFFSET_WIDTH) );
    return (pageByBit | raleventBits);
}
word_t find_empty_frame(word_t frame_address, int depth, int *max_frame_index, uint64_t *min_cyclic_distance,
                        uint64_t virtualAddressWithoutOffset, uint64_t pageByBit, word_t *minFrame,
                        uint64_t size_of_mini_address, uint64_t * pageEvictIndex, word_t father, int childIndex,
                        word_t* frameNotEvict,word_t* father_address) {
//    uint64_t current_mini_word =
//            (virtualAddressWithoutOffset >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * depth))
//             & ~(~0 << (size_of_mini_address + 1))); //msb
//    PMread((address_first  * PAGE_SIZE) + current_mini_word, &address_next);
    bool isAllZero = true;
    word_t return_frame = 0;
    bool flag = true;
    for (int i = 0; i < TABLES_DEPTH; i++) {
        word_t result = *(frameNotEvict + sizeof(word_t)*i);
        if (result == frame_address && depth != TABLES_DEPTH) {
            flag = false;
        }
    }
    word_t tmp;
    if (flag && depth <TABLES_DEPTH) {
        for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
            PMread(frame_address *PAGE_SIZE + i, &tmp);
            if (tmp != 0) {
                isAllZero = false;
                break;
            }
        }
        if (isAllZero && father != 0) { //TODO: if frame return 0
            PMwrite(father*PAGE_SIZE + childIndex, 0);
            return frame_address;
        }
    }

    if (depth == TABLES_DEPTH) {
//        bool flag = true;
//        for (int i = 0; i < TABLES_DEPTH; i++) {
//            if (*(frameNotEvict + sizeof(word_t)) == frame_address) {
//                flag = false;
//            }
//        }
        uint64_t distance = cyclic_distance(father, virtualAddressWithoutOffset);
        if (distance < *min_cyclic_distance && flag) {
            *min_cyclic_distance = distance;
            *minFrame = frame_address;
            *father_address = father *PAGE_SIZE + childIndex;
            *pageEvictIndex = pageByBit;
        }
        return -1;
    }//TODO: check edge case

    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame_address * PAGE_SIZE + i, &tmp);
/*
        if (*max_frame_index < tmp) {
            *max_frame_index = tmp;
        }
*/
        if (tmp != 0) {

            return_frame = find_empty_frame(tmp, depth + 1, max_frame_index, min_cyclic_distance,
                                            virtualAddressWithoutOffset,
                                            bitwiseAddTheNewFrameAddressBit(pageByBit, i, depth),
                                            minFrame, 0, pageEvictIndex, frame_address, i,
                                            frameNotEvict, father_address);
        }
        if (return_frame != -1 && return_frame != 0) {
            return return_frame;
        }
    }
    return -1;
}

int count_tree_size(word_t frame, int depth) {
    word_t tmp;
    int sum = 0;
//    if (depth == TABLES_DEPTH - 1){
//        PMread(frame * PAGE_SIZE, &tmp);
//        if (tmp != 0){
//            return 1;
//        }
//        return 0;
//    }
    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame * PAGE_SIZE + i, &tmp);
        if (tmp != 0 && depth < TABLES_DEPTH - 1) {
            sum += 1 + count_tree_size(tmp, depth + 1);
        }
        else if (tmp != 0 && depth == TABLES_DEPTH - 1)
            return 1;
    }
    return sum;
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
    uint64_t virtual_address_no_offset_length = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;
    uint64_t size_of_mini_address = virtual_address_no_offset_length / TABLES_DEPTH;
    uint64_t pageIndex =
            (virtualAddress >> OFFSET_WIDTH); //msb

    word_t address_first = 0;
    word_t address_next = 0;
    word_t finish_address;
    word_t minFrame = 0;
    uint64_t pageEvictIndex = 0;
    word_t father_address = 0;
    int max_frame_index = count_tree_size(0, 0);
    uint64_t min_cyclic_distance = NUM_PAGES;
    word_t framesNotEvict[TABLES_DEPTH];
    for (int i =0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = 0;
    }
    int cur_depth = 0;
    for (int i = 0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = address_first;
//        uint64_t current_mini_word =
//                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * (i+1))
//                 & ~(~0 << (size_of_mini_address + 1)))); //msb
        uint64_t current_mini_word = (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (OFFSET_WIDTH + (i * size_of_mini_address)))) & ((1L <<  OFFSET_WIDTH) - 1);
        PMread((address_first * PAGE_SIZE) + current_mini_word, &address_next);

        //framesNotEvict[i] = address_first; //TODO: you can't delete your parents
        //TODO: put zero in the table parents in case of taking the frame

        if (address_next == 0) {
            address_next = find_empty_frame(0, 0,
                                            &max_frame_index, &min_cyclic_distance,
                                            (virtualAddress >> OFFSET_WIDTH), 0, &minFrame,
                                            size_of_mini_address, &pageEvictIndex, 0, 0, framesNotEvict,
                                            &father_address);
            if (address_next != -1) {
                PMwrite(address_first*PAGE_SIZE+ current_mini_word, address_next);
            }
            else if (address_next == -1 && max_frame_index + 1 < NUM_FRAMES) {
                address_next = ++max_frame_index;
            } else if (address_next == -1) {
                address_next = minFrame;
                PMevict(address_next, pageEvictIndex);
                PMwrite(father_address, 0);
                cur_depth = -1;
                //PMwrite(address_first * PAGE_SIZE + current_mini_word, address_next);
//                for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
//                    PMwrite(address_next* PAGE_SIZE + k, 0);
//                }
            }
            //address_next = address_first;
            //swap out the frame
            if (i == TABLES_DEPTH - 1){
                PMrestore(address_next, pageIndex);
            }
            for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
                PMwrite(address_next*PAGE_SIZE +k, 0);
            }
            PMwrite(address_first*PAGE_SIZE +current_mini_word, address_next);
            address_first = address_next;
        }
        address_first = address_next;
        cur_depth++;
    }

    finish_address = address_first;
    uint64_t offset_mini_word =
            (virtualAddress >> (0)
             & ~(~0 << (OFFSET_WIDTH))); //TODO:check
    PMread((finish_address * PAGE_SIZE) + offset_mini_word, value);
    return 1;
}



/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value) {
    uint64_t virtual_address_no_offset_length = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;
    uint64_t size_of_mini_address = virtual_address_no_offset_length / TABLES_DEPTH;
    uint64_t pageIndex =
            (virtualAddress >> OFFSET_WIDTH); //msb

    word_t address_first = 0;
    word_t address_next = 0;
    word_t finish_address;
    word_t minFrame = 0;
    uint64_t pageEvictIndex = 0;
    word_t father_address = 0;
    int max_frame_index = count_tree_size(0, 0);
    uint64_t min_cyclic_distance = NUM_PAGES;
    word_t framesNotEvict[TABLES_DEPTH];
    for (int i =0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = 0;
    }
    int cur_depth = 0;
    for (int i = 0; i < TABLES_DEPTH; i++) {
        framesNotEvict[i] = address_first;
//        uint64_t current_mini_word =
//                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * (i+1))
//                 & ~(~0 << (size_of_mini_address + 1)))); //msb
        uint64_t current_mini_word = (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (OFFSET_WIDTH + (i * size_of_mini_address)))) & ((1L <<  OFFSET_WIDTH) - 1);
        PMread((address_first * PAGE_SIZE) + current_mini_word, &address_next);

        //framesNotEvict[i] = address_first; //TODO: you can't delete your parents
        //TODO: put zero in the table parents in case of taking the frame

        if (address_next == 0) {
            address_next = find_empty_frame(0, 0,
                                            &max_frame_index, &min_cyclic_distance,
                                            (virtualAddress >> OFFSET_WIDTH), 0, &minFrame,
                                            size_of_mini_address, &pageEvictIndex, 0, 0, framesNotEvict,
                                            &father_address);
            if (address_next != -1) {
                PMwrite(address_first*PAGE_SIZE+ current_mini_word, address_next);
            }
            else if (address_next == -1 && max_frame_index + 1 < NUM_FRAMES) {
                address_next = ++max_frame_index;
            } else if (address_next == -1) {
                address_next = minFrame;
                PMevict(address_next, pageEvictIndex);
                PMwrite(father_address, 0);
                cur_depth = -1;
                //PMwrite(address_first * PAGE_SIZE + current_mini_word, address_next);
//                for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
//                    PMwrite(address_next* PAGE_SIZE + k, 0);
//                }
            }
            //address_next = address_first;
            //swap out the frame
            if (i == TABLES_DEPTH - 1){
                PMrestore(address_next, pageIndex);
            }
            for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
                PMwrite(address_next*PAGE_SIZE +k, 0);
            }
            PMwrite(address_first*PAGE_SIZE +current_mini_word, address_next);
            address_first = address_next;
        }
        address_first = address_next;
        cur_depth++;
    }

    finish_address = address_first;
    uint64_t offset_mini_word =
            (virtualAddress >> (0)
             & ~(~0 << (OFFSET_WIDTH))); //TODO:check
    PMwrite((finish_address * PAGE_SIZE) + offset_mini_word, value);
    return 1;
    //PMwrite((finish_address * PAGE_SIZE) + offset_mini_word, value);
}

