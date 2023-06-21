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

uint64_t bitwiseAddTheNewFrameAddressBit(uint64_t pageByBit, word_t frame_address, int depth) {
    uint64_t raleventBits = (frame_address << (VIRTUAL_ADDRESS_WIDTH - ((depth + 1)* OFFSET_WIDTH) ));
    return pageByBit | raleventBits;
}
word_t find_empty_frame(word_t frame_address, int depth, int *max_frame_index, uint64_t *min_cyclic_distance,
                        uint64_t virtualAddressWithoutOffset, uint64_t pageByBit, word_t *minFrame,
                        uint64_t size_of_mini_address, uint64_t * pageEvictIndex, word_t father, int childIndex) {
//    uint64_t current_mini_word =
//            (virtualAddressWithoutOffset >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * depth))
//             & ~(~0 << (size_of_mini_address + 1))); //msb
//    PMread((address_first  * PAGE_SIZE) + current_mini_word, &address_next);
    word_t tmp;
    word_t return_frame = 0;
    *max_frame_index += 1;
    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame_address + i, &tmp);
        if (tmp != 0)
            break;
    }
    if (tmp == 0 && frame_address != 0) { //TODO: if frame return 0
        PMwrite(father + childIndex, 0);
        return -1;
    }
    if (depth > TABLES_DEPTH) {
        uint64_t distance = cyclic_distance(pageByBit, virtualAddressWithoutOffset);
        if (distance < *min_cyclic_distance) {
            *min_cyclic_distance = distance;
            *minFrame = frame_address;
            *pageEvictIndex = pageByBit;
        }
        return -1;
    }//TODO: check edge case

    for (int i = 0; i < (1L <<  OFFSET_WIDTH); i++) {
        PMread(frame_address + i, &tmp);
/*
        if (*max_frame_index < tmp) {
            *max_frame_index = tmp;
        }
*/
        if (tmp != 0) {
            return_frame = find_empty_frame(tmp, depth + 1, max_frame_index, min_cyclic_distance,
                                            virtualAddressWithoutOffset,
                                            bitwiseAddTheNewFrameAddressBit(pageByBit, i, depth),
                                            minFrame, 0, pageEvictIndex, frame_address, i);
        }
        if (return_frame != -1 && return_frame != 0) {
            return return_frame;
        }
    }
    return -1;
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
    uint64_t pageIndex =
            (virtualAddress >> OFFSET_WIDTH); //msb

    word_t address_first = 0;
    word_t address_next = 0;
    word_t finish_address;
    word_t minFrame = 0;
    uint64_t pageEvictIndex = 0;
    int max_frame_index = 0;
    uint64_t min_cyclic_distance = NUM_PAGES;
    uint64_t framesNotEvict[TABLES_DEPTH + 1];
    for (int i = 0; i < TABLES_DEPTH; i++) {
        max_frame_index++;
        framesNotEvict[i] = address_first;
        uint64_t current_mini_word =
                (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * i))
                 & ~(~0 << (size_of_mini_address + 1))); //msb
        PMread((address_first * PAGE_SIZE) + current_mini_word, &address_next);

        //framesNotEvict[i] = address_first; //TODO: you can't delete your parents
        //TODO: put zero in the table parents in case of taking the frame

        if (address_next == 0) {
            address_next = find_empty_frame(0, i,
                                             &max_frame_index, &min_cyclic_distance,
                                             virtualAddress, 0, &minFrame,
                                             size_of_mini_address, &pageEvictIndex, 0, 0);

            if (address_next == -1 && max_frame_index + 1 < NUM_FRAMES) {
                address_next = max_frame_index + 1;
            } else if (address_next == -1) {
                address_next = minFrame;
                PMevict(address_next, pageEvictIndex);
            }
            //address_next = address_first;
            //swap out the frame
            PMrestore(address_next, pageIndex);
            for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
                PMwrite(address_next*PAGE_SIZE +k, 0);
            }
            PMwrite(address_first*PAGE_SIZE +current_mini_word, address_next);
            address_first = address_next;
            break;
        }
    }

    finish_address = address_first;
    uint64_t offset_mini_word =
            (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * TABLES_DEPTH))
             & ~(~0 << (size_of_mini_address + 1))); //msb
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
  uint64_t virtual_address_no_offset = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;
  uint64_t size_of_mini_address = virtual_address_no_offset / TABLES_DEPTH;
  uint64_t pageIndex =
      (virtualAddress >> OFFSET_WIDTH); //msb

  word_t address_first = 0;
  word_t address_next = 0;
  word_t finish_address;
  word_t minFrame = 0;
  uint64_t pageEvictIndex = 0;
  int max_frame_index = 0;
  uint64_t min_cyclic_distance = NUM_PAGES;
  uint64_t framesNotEvict[TABLES_DEPTH + 1];
  for (int i = 0; i < TABLES_DEPTH; i++) {
      max_frame_index++;
      framesNotEvict[i] = address_first;
      uint64_t current_mini_word =
          (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * i))
           & ~(~0 << (size_of_mini_address + 1))); //msb
      PMread((address_first * PAGE_SIZE) + current_mini_word, &address_next);

      //framesNotEvict[i] = address_first; //TODO: you can't delete your parents
      //TODO: put zero in the table parents in case of taking the frame

      if (address_next == 0) {
          address_next = find_empty_frame(0, i,
                                          &max_frame_index, &min_cyclic_distance,
                                          virtualAddress, 0, &minFrame,
                                          size_of_mini_address, &pageEvictIndex, 0, 0);

          if (address_next == -1 && max_frame_index + 1 < NUM_FRAMES) {
              address_next = max_frame_index + 1;
            } else if (address_next == -1) {
              address_next = minFrame;
              PMevict(address_next, pageEvictIndex);
            }
          //address_next = address_first;
          //swap out the frame
          PMrestore(address_next, pageIndex);
          for (int k = 0; k < (1L <<  OFFSET_WIDTH); k++) {
              PMwrite(address_next*PAGE_SIZE +k, 0);
            }
          PMwrite(address_first*PAGE_SIZE +current_mini_word, address_next);
          address_first = address_next;
          break;
        }
        address_first =address_next;
    }

  finish_address = address_first;
  uint64_t offset_mini_word =
      (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (size_of_mini_address * TABLES_DEPTH))
       & ~(~0 << (size_of_mini_address + 1))); //msb
    PMwrite((finish_address * PAGE_SIZE) + offset_mini_word, value);
    return 1;
    //PMwrite((finish_address * PAGE_SIZE) + offset_mini_word, value);
}

