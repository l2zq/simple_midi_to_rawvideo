#include "mydefs.h"

class Note {
   public:
    Note* tck_next = NULL;
    Note *key_prev = NULL, *key_next = NULL;
    noteid_t note_id = -1;
    ui16 track = 0;
    ui08 channel = 0;
};
class NoteLinkedList {
   public:
    Note head, *tail = &head;
    Note* TckRmHead();            // returns removed item
    Note* KeyRemove(Note* item);  // returns head.key_next after removing
    void KeyAppend(Note* item);
    void TckAppend(Note* item);
};

using CKLinkedList = NoteLinkedList[16][128];

class MidiRoll {
   protected:
    bool use_pre = false;
    size_t pre_cnt = 0, pre_i = 0;
    Note *pre = NULL, **avails = NULL;
    Note* get();

   public:
    NoteLinkedList keys[128];
    CKLinkedList* tckeys;
    MidiRoll(ui16 track_count);
    MidiRoll(ui16 track_count, size_t pre_count);
    ~MidiRoll();
    void pre_free(Note* item);
    Note* KeyDn(ui16 track, ui08 channel, ui08 key);
    Note* KeyDn_pre(ui16 track, ui08 channel, ui08 key);
    Note* KeyUp(ui16 track, ui08 channel, ui08 key);
};