/*
 * translate.c - Offline Chinese<->English translation C module for MicroPython
 *
 * Built-in dictionary data is compiled into firmware as sorted C arrays.
 * Lookup uses binary search (O(log n)).
 *
 * Usage in MicroPython:
 *   import translate
 *   translate.lookup("hello")    # -> "你好"
 *   translate.lookup("你好")      # -> "hello"
 *   translate.en2zh("world")     # -> "世界"
 *   translate.zh2en("世界")       # -> "world"
 *   translate.lookup("xyz")      # -> None
 */

#include <stdio.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/misc.h"

#include "dict_data.h"

/* ------------------------------------------------------------------ */
/*  Binary search on a sorted table of {key, value} pairs             */
/* ------------------------------------------------------------------ */
static int tr_bsearch(const tr_entry_t *tbl, int n, const char *key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int c = strcmp(key, tbl[mid].key);
        if (c == 0) {
            return mid;
        } else if (c < 0) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/*  lookup(word) - auto-detect direction                              */
/*  (function names prefixed tr_func_ to avoid collision with the     */
/*   data arrays tr_en2zh[] / tr_zh2en[] from dict_data.h)           */
/* ------------------------------------------------------------------ */
static mp_obj_t tr_func_lookup(mp_obj_t word_obj) {
    const char *w = mp_obj_str_get_str(word_obj);

    int idx = tr_bsearch(tr_en2zh, TR_EN2ZH_COUNT, w);
    if (idx >= 0) {
        return mp_obj_new_str(tr_en2zh[idx].value, strlen(tr_en2zh[idx].value));
    }

    idx = tr_bsearch(tr_zh2en, TR_ZH2EN_COUNT, w);
    if (idx >= 0) {
        return mp_obj_new_str(tr_zh2en[idx].value, strlen(tr_zh2en[idx].value));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(tr_lookup_obj, tr_func_lookup);

/* ------------------------------------------------------------------ */
/*  en2zh(word) - English to Chinese                                   */
/* ------------------------------------------------------------------ */
static mp_obj_t tr_func_en2zh(mp_obj_t word_obj) {
    const char *w = mp_obj_str_get_str(word_obj);
    int idx = tr_bsearch(tr_en2zh, TR_EN2ZH_COUNT, w);
    if (idx >= 0) {
        return mp_obj_new_str(tr_en2zh[idx].value, strlen(tr_en2zh[idx].value));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(tr_en2zh_obj, tr_func_en2zh);

/* ------------------------------------------------------------------ */
/*  zh2en(word) - Chinese to English                                 */
/* ------------------------------------------------------------------ */
static mp_obj_t tr_func_zh2en(mp_obj_t word_obj) {
    const char *w = mp_obj_str_get_str(word_obj);
    int idx = tr_bsearch(tr_zh2en, TR_ZH2EN_COUNT, w);
    if (idx >= 0) {
        return mp_obj_new_str(tr_zh2en[idx].value, strlen(tr_zh2en[idx].value));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(tr_zh2en_obj, tr_func_zh2en);

/* ------------------------------------------------------------------ */
/*  count() - return total dictionary entries                        */
/* ------------------------------------------------------------------ */
static mp_obj_t tr_func_count(void) {
    return mp_obj_new_int(TR_EN2ZH_COUNT);
}
static MP_DEFINE_CONST_FUN_OBJ_0(tr_count_obj, tr_func_count);

/* ------------------------------------------------------------------ */
/*  Module registration                                               */
/* ------------------------------------------------------------------ */
static const mp_rom_map_elem_t translate_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_translate) },
    { MP_ROM_QSTR(MP_QSTR_lookup),   MP_ROM_PTR(&tr_lookup_obj)   },
    { MP_ROM_QSTR(MP_QSTR_en2zh),    MP_ROM_PTR(&tr_en2zh_obj)    },
    { MP_ROM_QSTR(MP_QSTR_zh2en),    MP_ROM_PTR(&tr_zh2en_obj)    },
    { MP_ROM_QSTR(MP_QSTR_count),    MP_ROM_PTR(&tr_count_obj)    },
};
static MP_DEFINE_CONST_DICT(translate_module_globals, translate_module_globals_table);

const mp_obj_module_t translate_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&translate_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_translate, translate_user_cmodule);
