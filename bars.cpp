#include "bars.h"
#include <stdio.h>

void KeyBars::AddBar(Bar* nbar, tick_t time, bool end_old, bool beg_new) {
    nbar->next = NULL;
    tail->next = nbar;
    tail->bar_end = nbar->bar_beg = time;
    if (end_old)
        tail->note_end = time;
    if (beg_new)
        nbar->note_beg = time;
    tail = nbar;
}
Bar* KeyBars::DelBef(tick_t time) {
    Bar *del_beg = head.next, *ptr = head.next, *del_end = NULL;
    while (ptr) {
        if (ptr->bar_end <= time) {
            del_end = ptr;
            ptr = ptr->next;
        } else
            break;
    }
    if (del_end) {
        if ((head.next = del_end->next) == NULL)
            tail = &head;
        del_end->next = NULL;
        return del_beg;
    } else
        return NULL;
}

Bar* MidiBars::AddBar(ui08 key, tick_t time, bool end_old, bool beg_new) {
    Bar* nbar = new Bar;
    kbar[key].AddBar(nbar, time, end_old, beg_new);
    return nbar;
}
Bar* MidiBars::AddBar_pre(ui08 key, tick_t time, bool end_old, bool beg_new) {
    Bar* nbar = get();
    kbar[key].AddBar(nbar, time, end_old, beg_new);
    return nbar;
}

void MidiBars::DelBef(tick_t time) {
    Bar *bar, *del;
    for (ui08 key = 0; key < 128; key++) {
        bar = kbar[key].DelBef(time);
        while (bar) {
            bar = (del = bar)->next;
            delete del;
        }
    }
}
void MidiBars::DelBef_pre(tick_t time) {
    Bar *bar, *del;
    for (ui08 key = 0; key < 128; key++) {
        bar = kbar[key].DelBef(time);
        while (bar) {
            bar = (del = bar)->next;
            pre_free(del);
        }
    }
}
MidiBars::~MidiBars() {
    if (use_pre) {
        delete[] pre;
        delete[] avails;
    } else {
        Bar *bar, *del;
        for (ui08 key = 0; key < 128; key++) {
            bar = kbar[key].head.next;
            while (bar) {
                bar = (del = bar)->next;
                delete del;
            }
        }
    }
}

MidiBars::MidiBars() {
}
MidiBars::MidiBars(size_t pre_count) {
    use_pre = true;
    pre = new Bar[pre_i = pre_cnt = pre_count];
    avails = new Bar*[pre_count];
    for (size_t i = 0; i < pre_count; i++)
        avails[i] = pre + i;
}
Bar* MidiBars::get() {
    Bar* bar;
    if (pre_i) {
        bar = avails[--pre_i];
        bar->bar_end = TICK_T_MAX;
        return bar;
    }
    fprintf(stderr, "MidiBars::get() == NULL!\n");
    return NULL;
}
void MidiBars::pre_free(Bar* bar) {
    avails[pre_i++] = bar;
}