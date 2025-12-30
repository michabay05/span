#ifndef _UMKA_H_
#define _UMKA_H_

#include "umka_api.h"

typedef UmkaDynArray(DVector2) UmkaCurvePts;
typedef struct {
    const char *name;
    UmkaExternFunc func;
} UmkaFunc;

bool spu_init(const char *filename);
void spu_run_sequence(void);
void spu_preamble_count_lines(const char *preamble);
void spu_print_err(void);
bool spu_call_fn(const char *fn_name, UmkaStackSlot **slot, size_t storage_bytes);
bool spu_content_w_preamble(const char *filename, char **content);
void spuo_rect(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_text(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_axes(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_curve(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_typst(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_get_camera(UmkaStackSlot *p, UmkaStackSlot *r);
void spuo_enable(UmkaStackSlot *p, UmkaStackSlot *r);
void spu_fade_in(UmkaStackSlot *p, UmkaStackSlot *r);
void spu_fade_out(UmkaStackSlot *p, UmkaStackSlot *r);
void spu_wait(UmkaStackSlot *p, UmkaStackSlot *r);
void spu_move(UmkaStackSlot *p, UmkaStackSlot *r);
void spu_play(UmkaStackSlot *p, UmkaStackSlot *r);

#endif // _UMKA_H_
