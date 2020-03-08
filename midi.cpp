#include "midi.h"

MidiFile::MidiFile(const char* filename) {
    int fd;
    struct stat buf;
    if ((fd = open(filename, O_RDONLY)) == -1)
        return;
    if (fstat(fd, &buf) != -1                                                                           //
        && (map = (ui08*)mmap(NULL, f_len = buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) != MAP_FAILED  //
    ) {
        if (!PostOpen())
            munmap(map, f_len);
    }
    close(fd);
    return;
}
MidiFile::~MidiFile() {
    if (open_ok) {
        delete[] tracks;
        munmap(map, f_len);
    }
}

bool MidiFile::PostOpen() {
    ui32 t_len;
    ui08 *mem = map + 14, *lim = mem + f_len - 8;
    if (f_len < 14 || mem_be32(map) != 0x4d546864)
        return false;
    tracks = new MidiTrack[track_count = mem_be16(map + 10)];
    for (ui16 i = 0; i < track_count && mem < lim; i++) {
        if (mem_be32(mem) != 0x4d54726b) {
            delete[] tracks;
            return false;
        }
        tracks[i].setptrs(mem + 8, t_len = mem_be32(mem + 4));
        mem += 8 + t_len;
    }
    time_division = mem_be16(map + 12);
    return open_ok = true;
}

bool MidiFile::ok() {
    return open_ok;
}

void MidiTrack::setptrs(ui08* _mem, ui32 len) {
    not_end = len > 0;
    end = (ptr = mem = _mem) + len;
}

ui32 MidiTrack::vln() {
    ui08 c;
    ui32 d = (c = *(ptr++)) & 0x7f;
    if (c & 0x80) {
        d = (d << 7) | ((c = *(ptr++)) & 0x7f);
        if (c & 0x80) {
            d = (d << 7) | ((c = *(ptr++)) & 0x7f);
            if (c & 0x80)
                d = (d << 7) | (*(ptr++) & 0x7f);
        }
    }
    return d;
}

ui32 MidiTrack::next_det() {
    if (not_end && isdet) {
        isdet = false;
        return vln();
    }
    return __INT_MAX__;
}
bool MidiTrack::next_evt(MidiEvent* evt) {
    if (isdet)
        return false;
    ui08 cmd;
    ui32 len;
    switch (cmd = *(ptr++)) {
        case 0xFF:
            evt->ms_evt.type = *(ptr++);
        case 0xF0:
            evt->ms_evt.len = len = vln();
            evt->ms_evt.data = ptr;
            ptr += len;
            evt->evt.other = true;
            break;
        default: {
            evt->evt.other = false;
            if (cmd & 0x80) {
                rs = cmd;
                evt->evt.arg1 = *(ptr++);
            } else {
                evt->evt.arg1 = cmd;
                cmd = rs;
            }
            switch (cmd >> 4) {
                case 0x8:
                case 0x9:
                case 0xA:
                case 0xB:
                case 0xE:
                    evt->evt.arg2 = *(ptr++);
            }
        }
    }
    evt->cmd = cmd;
    isdet = true;
    return not_end = (ptr < end);
}