#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include "bars.h"
#include "midi.h"
#include "roll.h"

// ffplay -f rawvideo -pixel_format bgra -s WxH -i -

ui16 frame_w = 640, frame_h = 480, fps = 30;
ui32 max_polyphony = 32768;
float vp_height = 1.0;

#define FRAME_W frame_w
#define FRAME_H frame_h
#define FRAMERATE fps
#define PRESCROLL_TICKS 0  //((tick_t)time_division * 0)
#define VIEWPORT_HEIGHT vp_height;
#define BGRA(R, G, B) (0xFF000000 + ((R) << 16) + ((G) << 8) + (B))

// const ui32 backgrnd = BGRA(0xD0, 0xD0, 0xFF);
const ui32 backgrnd = BGRA(0xFF, 0xFF, 0xFF);
const ui32 bordrclr = BGRA(0x00, 0x00, 0x00);
const ui32 colors[] = {                                                      //
    BGRA(0xFF, 0x00, 0x00), BGRA(0x00, 0xFF, 0x00), BGRA(0x00, 0x00, 0xFF),  //
    BGRA(0xFF, 0xFF, 0x00), BGRA(0x00, 0xFF, 0xFF), BGRA(0xFF, 0x00, 0xFF),  //
    BGRA(0x7F, 0x00, 0x00), BGRA(0x00, 0x7F, 0x00), BGRA(0x00, 0x00, 0x7F),  //
    BGRA(0x7F, 0x7F, 0x00), BGRA(0x00, 0x7F, 0x7F), BGRA(0x7F, 0x00, 0x7F),  //
    BGRA(0x7F, 0xFF, 0x00), BGRA(0x00, 0x7F, 0xFF), BGRA(0x7F, 0x00, 0xFF),  //
    BGRA(0xFF, 0x7F, 0x00), BGRA(0x00, 0xFF, 0x7F), BGRA(0xFF, 0x00, 0x7F)};
const ui32 ncolors = sizeof(colors) / sizeof(ui32);
#define MAX_POLYPHONY max_polyphony

typedef struct {
    tick_t time;
    ui32 new_mpqn;
} SetMPQN;

class MidiWhat {
   public:
    std::queue<SetMPQN> mpqn_queue;
    MidiFile* mf = NULL;
    MidiRoll* mr = NULL;
    MidiBars* mb = NULL;
    ui32* last_delta = NULL;
    // dynamically alocated above
    MidiTrack* trks = NULL;
    bool ok = false, end = false;
    ui16 ntrk = 0, time_division;
    ui32 mpqn = 500000;            // default to 120bpm, or 0.5s/qn
    noteid_t processed_notes = 0;  // noteid
    tick_t processed_ticks = 0;
    tick_t viewport_height;
    //
    void Begin(tick_t begin_time) {
        processed_ticks = begin_time;
        for (ui16 i = 0; i < ntrk; i++)
            last_delta[i] = trks[i].next_det();
    }
    tick_t RunToTime(tick_t wanted_time) {
        if (end)
            return processed_ticks;
        ui08 k;
        ui16 i;
        ui32 mindet;
        MidiEvent evt;
        Bar* bar;
        Note* note;
        noteid_t nid_tmp, key_noteids[128];
        bool end_old[128] = {false}, beg_new[128] = {false};
        while (processed_ticks < wanted_time) {
            mindet = __INT32_MAX__;
            for (k = 0; k < 128; k++)
                if ((note = mr->keys[k].head.key_next) == NULL)
                    key_noteids[k] = -1;
                else
                    key_noteids[k] = note->note_id;
            for (i = 0; i < ntrk; i++) {
                while (last_delta[i] == 0) {
                    if (trks[i].next_evt(&evt))
                        last_delta[i] = trks[i].next_det();
                    else  // save a call to next_det when after event it's end
                        last_delta[i] = __INT32_MAX__;
                    switch (evt.cmd >> 4) {
                        case 0x8:  // key up
                            note = mr->KeyUp(i, evt.cmd & 0xF, k = evt.evt.arg1);
                            if (note) {
                                if (note->note_id == key_noteids[k])
                                    end_old[k] = true;
                                mr->pre_free(note);
                            }
                            break;
                        case 0x9:  // key dn
                            note = mr->KeyDn_pre(i, evt.cmd & 0xF, k = evt.evt.arg1);
                            note->note_id = (processed_notes++);
                            beg_new[k] = true;
                            break;
                    }
                    if (evt.cmd == 0xFF && evt.ms_evt.type == 0x51) {
                        ui08* data = evt.ms_evt.data;
                        ui08 m[4];
                        m[0] = 0;
                        m[1] = data[0];
                        m[2] = data[1];
                        m[3] = data[2];
                        ui32 new_mpqn = mem_be32(m);
                        mpqn_queue.push({processed_ticks, new_mpqn});
                    }
                }
                if (last_delta[i] < mindet)  // we don't check whether its end, because INT32MAX can't be a det
                    mindet = last_delta[i];
            }
            for (k = 0; k < 128; k++) {
                if ((note = mr->keys[k].head.key_next) == NULL)
                    nid_tmp = -1;
                else
                    nid_tmp = note->note_id;
                if (nid_tmp != key_noteids[k]) {
                    bar = mb->AddBar_pre(k, processed_ticks, end_old[k], beg_new[k]);
                    // bar = mb->AddBar(k, processed_ticks, end_old[k], beg_new[k]);
                    bar->noteid = nid_tmp;
                    if (note) {
                        bar->track = note->track;
                        bar->channel = note->channel;
                    }
                }
                end_old[k] = false;
                beg_new[k] = false;
            }
            // time elapse
            if (mindet == __INT32_MAX__) {  // all ends
                end = true;
                return processed_ticks;
            } else {
                processed_ticks += mindet;
                for (i = 0; i < ntrk; i++)
                    if (last_delta[i] != __INT32_MAX__)
                        last_delta[i] -= mindet;
            }
        }
        return processed_ticks;
    }
    ~MidiWhat() {
        if (ok) {
            delete mb;
            delete mr;
            delete mf;
            delete[] last_delta;
        }
    }
    MidiWhat(const char* filename) {
        mf = new MidiFile(filename);
        if (mf->ok()) {
            fprintf(stderr, "track_count: %d, time_division: %d\n", mf->track_count, mf->time_division);

            time_division = mf->time_division;
            viewport_height = time_division * vp_height;

            ok = true;
            trks = mf->tracks;
            last_delta = new ui32[ntrk = mf->track_count];
            mr = new MidiRoll(ntrk, MAX_POLYPHONY);
            // mb = new MidiBars();
            mb = new MidiBars(viewport_height * 128);
        } else
            delete mf;
    }
    void Main();
};

int main(int argc, char** argv) {
    if (argc < 2)
        return 1;
    if (argc > 3) {
        frame_w = atoi(argv[2]);
        frame_h = atoi(argv[3]);
        if (argc > 4)
            fps = atoi(argv[4]);
        if (argc > 5)
            vp_height = atof(argv[5]);
        if (argc > 6)
            max_polyphony = atoi(argv[6]);
        fprintf(stderr, "%dx%d %d %.3f %d\n", frame_w, frame_h, fps, vp_height, max_polyphony);
    }
    MidiWhat mw(argv[1]);
    if (mw.ok)
        mw.Main();
    else
        return 2;
    return 0;
}

void MidiWhat::Main() {
    si64 play_time_us = 0;
    si64 next_midi_us = 0, next_frame_us = 0;
    tick_t bottom_tick, top_tick;

    bottom_tick = -PRESCROLL_TICKS;
    top_tick = bottom_tick + viewport_height;

    Bar* bar;
    ui16 k;
    si32 x, y, l, r, u, d;
    tick_t b_u, b_d;
    ui32 *frame = new ui32[(FRAME_H+5) * FRAME_W], color;
    for (ui32 i = 0; i < FRAME_H * FRAME_W; i++)
        frame[i] = backgrnd;
    ui32* ys = new ui32[viewport_height + 1];
    for (b_u = 0; b_u <= viewport_height; b_u++)
        ys[b_u] = FRAME_H - b_u * FRAME_H / viewport_height;
    bool midi = false;
    Begin(0);
    RunToTime(top_tick);
    while (bottom_tick < processed_ticks) {
        midi = (play_time_us == next_midi_us);
        if (midi) {
            RunToTime(top_tick);
            mb->DelBef_pre(bottom_tick);
            // mb->DelBef(play_time_tick);
            // process set tempo events
            while (mpqn_queue.size()) {
                SetMPQN* set_mpqn = &mpqn_queue.front();
                if (set_mpqn->time > bottom_tick)
                    break;
                else {
                    mpqn = set_mpqn->new_mpqn;
                    // fprintf(stderr, "set tempo = %.3f\n", 60000000.0 / mpqn);
                }
                mpqn_queue.pop();
            }
            next_midi_us += mpqn;  // / time_division;
        }
        if (play_time_us == next_frame_us) {
            next_frame_us += 1000000 * time_division / FRAMERATE;
            for (k = 0; k < 128; k++) {
                // break;  //!!!!
                l = k * FRAME_W / 128;
                r = (k + 1) * FRAME_W / 128;
                for (bar = mb->kbar[k].head.next; bar != NULL; bar = bar->next) {
                    if (bar->noteid == -1)
                        color = backgrnd;
                    else
                        color = colors[bar->track % ncolors];
                    if ((b_d = bar->bar_beg - bottom_tick) < 0)
                        b_d = 0;
                    if (bar->bar_end > top_tick)
                        b_u = viewport_height;
                    else
                        b_u = bar->bar_end - bottom_tick;
                    d = ys[b_d];
                    u = ys[b_u];
                    for (y = u; y < d; y++)
                        for (x = l; x < r; x++)
                            frame[y * FRAME_W + x] = color;
                    if(bar->noteid != -1)
                        for(y = u; y < d; y++){
                            frame[y * FRAME_W + l] = bordrclr;
                            frame[y * FRAME_W + r-1] = bordrclr;
                        }
                    if(bar->note_beg > bottom_tick)
                        if(bar->note_beg == bar->bar_beg)
                            for (x = l; x < r; x++)
                                frame[(d-1) * FRAME_W + x] = bordrclr;
                    if(bar->bar_end < top_tick)
                        if(bar->note_end == bar->bar_end)
                            for (x = l; x < r; x++)
                                frame[u * FRAME_W + x] = bordrclr;
                }
            }
            
            if (write(STDOUT_FILENO, frame, FRAME_H * FRAME_W * sizeof(ui32)) == -1) {
                fprintf(stderr, "MidiWhat: write() == -1 !\n");
                break;
            }
        }
        if (midi) {
            bottom_tick++;
            top_tick = bottom_tick + viewport_height;
        }
        // time advance
        play_time_us = next_frame_us > next_midi_us ? next_midi_us : next_frame_us;
    }
    delete[] ys;
    delete[] frame;
}