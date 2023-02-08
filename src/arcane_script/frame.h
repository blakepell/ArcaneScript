#ifndef frame_h
#define frame_h

#include <stdint.h>

#ifndef ARCANE_AMALGAMATED
#include "common.h"
#include "object.h"
#include "collections.h"
#include "code.h"
#endif

typedef struct
{
	object_t function;
	int ip;
	int base_pointer;
	const src_pos_t* src_positions;
	uint8_t* bytecode;
	int src_ip;
	int bytecode_size;
	int recover_ip;
	bool is_recovering;
} frame_t;

ARCANE_INTERNAL bool frame_init(frame_t* frame, object_t function, int base_pointer);

ARCANE_INTERNAL opcode_val_t frame_read_opcode(frame_t* frame);
ARCANE_INTERNAL uint64_t frame_read_uint64(frame_t* frame);
ARCANE_INTERNAL uint16_t frame_read_uint16(frame_t* frame);
ARCANE_INTERNAL uint8_t frame_read_uint8(frame_t* frame);
ARCANE_INTERNAL src_pos_t frame_src_position(const frame_t* frame);

#endif /* frame_h */
