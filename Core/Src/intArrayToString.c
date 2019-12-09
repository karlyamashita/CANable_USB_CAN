#include "IntArrayToString.h"
#include <stdio.h>

int UintArrayToString(uint8_t const* data, int data_length, char* output, int output_length) {
	int written = 0;
	  for (; data_length; data_length--) {
	    int length = snprintf(output, output_length, "%d", *data++);
	    if (length >= output_length) {
	      // not enough space
	      return -1;
	    }
	    written += length;
	    output += length;
	    output_length -= length;
	  }
	  return written;
}


