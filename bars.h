#include "mydefs.h"

class Bar {
   public:
    Bar* next = NULL;
    tick_t bar_beg = 0, bar_end = TICK_T_MAX;
    tick_t note_beg = 0, note_end = TICK_T_MAX;
    noteid_t noteid = -1;
    ui16 track;
    ui08 channel;
};

class KeyBars {
   public:
    noteid_t last_noteid = -1;
    Bar head, *tail = &head;
    Bar* DelBef(tick_t time);  // if have, return a new head to delete, if no, return NULL
    void AddBar(Bar* nbar, tick_t time, bool end_old, bool beg_new);
};

class MidiBars {
   protected:
    bool use_pre = false;
    size_t pre_cnt = 0, pre_i = 0;
    Bar *pre = NULL, **avails = NULL;
    Bar* get();
    void pre_free(Bar* bar);

   public:
    KeyBars kbar[128];
    MidiBars();
    MidiBars(size_t pre_count);
    ~MidiBars();
    void DelBef(tick_t time);
    void DelBef_pre(tick_t time);
    Bar* AddBar(ui08 key, tick_t time, bool end_old, bool beg_new);
    Bar* AddBar_pre(ui08 key, tick_t time, bool end_old, bool beg_new);
};