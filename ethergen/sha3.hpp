#ifndef SHA3_H
#define SHA3_H

#include <stdint.h>
#include <stdlib.h>

void SHA3_256(uint8_t* ret, uint8_t const* data, size_t const size);
void SHA3_512(uint8_t* ret, uint8_t const* data, size_t const size);

#endif
