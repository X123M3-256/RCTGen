#include <stdint.h>
#include "sprites.h"

uint8_t flat_pixels[]={1,2,3,1,2,3,3,1,2,3,1,2,2,3,1,2,3,1, 1,2,3,1,2,3,2,3,1,2,3,1,3,1,2,3,1,2, 1,3,2,1,3,2,2,1,3,2,1,3,3,2,1,3,2,1, 1,3,2,1,3,2,3,2,1,3,2,1,2,1,3,2,1,3};
image_t flat_chain[4]={{3,6,0,-2,flat_pixels},{3,6,0,-1,flat_pixels+18},{3,6,0,0,flat_pixels+36},{3,6,-1,0,flat_pixels+54}};

uint8_t gentle_pixels[]={1,1,2,2,3,3,3,3,1,1,2,2,2,2,3,3,1,1, 1,2,3,3,2,1, 2,2,1,1,3,3,1,1,3,3,2,2,3,3,2,2,1,1};
image_t gentle_chain[4]={{6,3,-3,-1,gentle_pixels},{3,1,1,0,gentle_pixels+18},{3,1,1,0,gentle_pixels+21},{6,3,0,-1,gentle_pixels+24}};

uint8_t flat_diag_chain_pixels[]={1,2,3,3,2,1};
image_t flat_diag_chain[4]={{3,1,-2,0,flat_diag_chain_pixels},{1,3,0,-2,flat_diag_chain_pixels},{3,1,-1,0,flat_diag_chain_pixels+3},{1,3,0,-1,flat_diag_chain_pixels+3}};
