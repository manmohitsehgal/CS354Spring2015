#define timeSlice 60

struct ts_ent{
    int ts_tqexp;
    int ts_slpret;
    int ts_quantum;
};
extern struct ts_ent tstab[];
