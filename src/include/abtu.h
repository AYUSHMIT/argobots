/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

#ifndef ABTU_H_INCLUDED
#define ABTU_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "abt_config.h"

/* Utility feature */
#ifdef ABT_CONFIG_HAVE___BUILTIN_EXPECT
#define ABTU_likely(cond)       __builtin_expect(!!(cond), 1)
#define ABTU_unlikely(cond)     __builtin_expect(!!(cond), 0)
#else
#define ABTU_likely(cond)       (cond)
#define ABTU_unlikely(cond)     (cond)
#endif

/* Utility Functions */

static inline
void *ABTU_malloc(size_t size)
{
    return malloc(size);
}

static inline
void *ABTU_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

static inline
void ABTU_free(void *ptr)
{
    free(ptr);
}

static inline
void *ABTU_realloc(void *ptr, size_t old_size, size_t new_size)
{
    (void)old_size;
    return realloc(ptr, new_size);
}

static inline
void *ABTU_memalign(size_t alignment, size_t size)
{
    void *p_ptr;
    int ret = posix_memalign(&p_ptr, alignment, size);
    assert(ret == 0);
    return p_ptr;
}

#define ABTU_strcpy(d,s)        strcpy(d,s)
#define ABTU_strncpy(d,s,n)     strncpy(d,s,n)

/* The caller should free the memory returned. */
char *ABTU_get_indent_str(int indent);

int ABTU_get_int_len(size_t num);
char *ABTU_strtrim(char *str);

#endif /* ABTU_H_INCLUDED */
