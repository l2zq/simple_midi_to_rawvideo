#include "roll.h"
#include <stdio.h>

void NoteLinkedList::KeyAppend(Note* item) {
    item->key_next = NULL;
    item->key_prev = tail;
    tail->key_next = item;
    tail = item;
}
void NoteLinkedList::TckAppend(Note* item) {
    item->tck_next = NULL;
    tail->tck_next = item;
    tail = item;
}
Note* NoteLinkedList::TckRmHead() {
    Note* item;
    if ((item = head.tck_next) != NULL)
        if ((head.tck_next = item->tck_next) == NULL)
            tail = &head;
    return item;
}
Note* NoteLinkedList::KeyRemove(Note* item) {
    if ((item->key_prev->key_next = item->key_next) != NULL)
        item->key_next->key_prev = item->key_prev;
    else
        tail = item->key_prev;
    return head.key_next;
}

MidiRoll::MidiRoll(ui16 track_count) {
    tckeys = new CKLinkedList[track_count];
}
MidiRoll::MidiRoll(ui16 track_count, size_t pre_count) {
    use_pre = true;
    pre = new Note[pre_i = pre_cnt = pre_count];
    avails = new Note*[pre_count];
    for (size_t i = 0; i < pre_count; i++)
        avails[i] = pre + i;
    tckeys = new CKLinkedList[track_count];
}
Note* MidiRoll::get() {
    if (pre_i)
        return avails[--pre_i];
    fprintf(stderr, "MidiRoll::get() == NULL!\n");
    return NULL;  // let it crash
}
void MidiRoll::pre_free(Note* item) {
    avails[pre_i++] = item;
}

MidiRoll::~MidiRoll() {
    if (use_pre) {
        delete[] pre;
        delete[] avails;
    } else {
        Note *note, *del;
        for (ui08 key = 0; key < 128; key++) {
            note = keys[key].head.key_next;
            while (note) {
                note = (del = note)->key_next;
                delete del;
            }
        }
    }
    delete[] tckeys;
}
Note* MidiRoll::KeyDn(ui16 track, ui08 channel, ui08 key) {
    Note* note = new Note;
    keys[key].KeyAppend(note);
    tckeys[track][channel][key].TckAppend(note);
    note->track = track;
    note->channel = channel;
    return note;
}
Note* MidiRoll::KeyDn_pre(ui16 track, ui08 channel, ui08 key) {
    Note* note = get();
    keys[key].KeyAppend(note);
    tckeys[track][channel][key].TckAppend(note);
    note->track = track;
    note->channel = channel;
    return note;
}
Note* MidiRoll::KeyUp(ui16 track, ui08 channel, ui08 key) {
    Note* note;
    if ((note = tckeys[track][channel][key].TckRmHead()) != NULL)
        keys[key].KeyRemove(note);
    return note;
}