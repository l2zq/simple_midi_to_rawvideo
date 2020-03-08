#pragma once

#include <endian.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mydefs.h"

#define mem_be16(mem) be16toh(*(ui16 *)(mem))
#define mem_be32(mem) be32toh(*(ui32 *)(mem))

typedef union {
    ui08 cmd;
    struct {
        ui08 cmd;
        ui08 arg1;
        ui08 arg2;
        bool other;
    } evt;
    struct {
        ui08 cmd;
        ui08 type;
        ui32 len;
        ui08 *data;
    } ms_evt;
} MidiEvent;

class MidiTrack {
   protected:
    bool isdet = true;
    ui08 rs = 0, *mem = NULL, *end = NULL, *ptr = NULL;
    ui32 vln();

   public:
    bool not_end = false;
    void setptrs(ui08 *_mem, ui32 len);
    ui32 next_det();
    bool next_evt(MidiEvent *evt);
};

class MidiFile {
   protected:
    bool PostOpen();
    bool open_ok = false;
    ui08 *map = NULL;
    size_t f_len = 0;

   public:
    MidiTrack *tracks = NULL;
    ui16 track_count = 0, time_division = 120;
    bool ok();
    MidiFile(const char *filename);
    ~MidiFile();
};