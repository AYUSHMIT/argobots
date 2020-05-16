/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

#ifndef ABTI_POOL_H_INCLUDED
#define ABTI_POOL_H_INCLUDED

/* Inlined functions for Pool */

static inline ABTI_pool *ABTI_pool_get_ptr(ABT_pool pool)
{
#ifndef ABT_CONFIG_DISABLE_ERROR_CHECK
    ABTI_pool *p_pool;
    if (pool == ABT_POOL_NULL) {
        p_pool = NULL;
    } else {
        p_pool = (ABTI_pool *)pool;
    }
    return p_pool;
#else
    return (ABTI_pool *)pool;
#endif
}

static inline ABT_pool ABTI_pool_get_handle(ABTI_pool *p_pool)
{
#ifndef ABT_CONFIG_DISABLE_ERROR_CHECK
    ABT_pool h_pool;
    if (p_pool == NULL) {
        h_pool = ABT_POOL_NULL;
    } else {
        h_pool = (ABT_pool)p_pool;
    }
    return h_pool;
#else
    return (ABT_pool)p_pool;
#endif
}

/* A ULT is blocked and is waiting for going back to this pool */
static inline void ABTI_pool_inc_num_blocked(ABTI_pool *p_pool)
{
    ABTD_atomic_fetch_add_int32(&p_pool->num_blocked, 1);
}

/* A blocked ULT is back in the pool */
static inline void ABTI_pool_dec_num_blocked(ABTI_pool *p_pool)
{
    ABTD_atomic_fetch_sub_int32(&p_pool->num_blocked, 1);
}

/* The pool will receive a migrated ULT */
static inline void ABTI_pool_inc_num_migrations(ABTI_pool *p_pool)
{
    ABTD_atomic_fetch_add_int32(&p_pool->num_migrations, 1);
}

/* The pool has received a migrated ULT */
static inline void ABTI_pool_dec_num_migrations(ABTI_pool *p_pool)
{
    ABTD_atomic_fetch_sub_int32(&p_pool->num_migrations, 1);
}

#ifdef ABT_CONFIG_DISABLE_POOL_PRODUCER_CHECK
static inline void ABTI_pool_push(ABTI_pool *p_pool, ABT_unit unit)
{
    LOG_EVENT_POOL_PUSH(p_pool, unit,
                        ABTI_self_get_native_thread_id(ABTI_local_get_xstream()));

    /* Push unit into pool */
    p_pool->p_push(ABTI_pool_get_handle(p_pool), unit);
}

static inline void ABTI_pool_add_thread(ABTI_thread *p_thread)
{
    /* Set the ULT's state as READY. The relaxed version is used since the state
     * is synchronized by the following pool operation. */
    ABTD_atomic_relaxed_store_int(&p_thread->state, ABT_THREAD_STATE_READY);

    /* Add the ULT to the associated pool */
    ABTI_pool_push(p_thread->p_pool, p_thread->unit);
}

#define ABTI_POOL_PUSH(p_pool, unit, p_producer) ABTI_pool_push(p_pool, unit)

#define ABTI_POOL_ADD_THREAD(p_thread, p_producer)                             \
    ABTI_pool_add_thread(p_thread)

#else /* ABT_CONFIG_DISABLE_POOL_PRODUCER_CHECK */

static inline int ABTI_pool_push(ABTI_pool *p_pool, ABT_unit unit,
                                 ABTI_native_thread_id producer_id)
{
    int abt_errno = ABT_SUCCESS;

    LOG_EVENT_POOL_PUSH(p_pool, unit, producer_id);

    /* Save the producer ES information in the pool */
    abt_errno = ABTI_pool_set_producer(p_pool, producer_id);
    ABTI_CHECK_ERROR(abt_errno);

    /* Push unit into pool */
    p_pool->p_push(ABTI_pool_get_handle(p_pool), unit);

fn_exit:
    return abt_errno;

fn_fail:
    HANDLE_ERROR_FUNC_WITH_CODE(abt_errno);
    goto fn_exit;
}

static inline int ABTI_pool_add_thread(ABTI_thread *p_thread,
                                       ABTI_native_thread_id producer_id)
{
    int abt_errno;

    /* Set the ULT's state as READY. The relaxed version is used since the state
     * is synchronized by the following pool operation. */
    ABTD_atomic_relaxed_store_int(&p_thread->state, ABT_THREAD_STATE_READY);

    /* Add the ULT to the associated pool */
    abt_errno = ABTI_pool_push(p_thread->p_pool, p_thread->unit, producer_id);
    ABTI_CHECK_ERROR(abt_errno);

fn_exit:
    return abt_errno;

fn_fail:
    HANDLE_ERROR_FUNC_WITH_CODE(abt_errno);
    goto fn_exit;
}

#define ABTI_POOL_PUSH(p_pool, unit, producer_id)                              \
    do {                                                                       \
        abt_errno = ABTI_pool_push(p_pool, unit, producer_id);                 \
        ABTI_CHECK_ERROR_MSG(abt_errno, "ABTI_pool_push");                     \
    } while (0)

#define ABTI_POOL_ADD_THREAD(p_thread, producer_id)                            \
    do {                                                                       \
        abt_errno = ABTI_pool_add_thread(p_thread, producer_id);               \
        ABTI_CHECK_ERROR(abt_errno);                                           \
    } while (0)

#endif /* ABT_CONFIG_DISABLE_POOL_PRODUCER_CHECK */

#ifdef ABT_CONFIG_DISABLE_POOL_CONSUMER_CHECK
static inline int ABTI_pool_remove(ABTI_pool *p_pool, ABT_unit unit)
{
    int abt_errno = ABT_SUCCESS;

    LOG_EVENT_POOL_REMOVE(p_pool, unit,
                          ABTI_self_get_native_thread_id(
                              ABTI_local_get_xstream()));

    abt_errno = p_pool->p_remove(ABTI_pool_get_handle(p_pool), unit);
    ABTI_CHECK_ERROR(abt_errno);

fn_exit:
    return abt_errno;

fn_fail:
    HANDLE_ERROR_FUNC_WITH_CODE(abt_errno);
    goto fn_exit;
}

#define ABTI_POOL_REMOVE(p_pool, unit, consumer_id)                            \
    ABTI_pool_remove(p_pool, unit)
#define ABTI_POOL_SET_CONSUMER(p_pool, consumer_id)

#else /* ABT_CONFIG_DISABLE_POOL_CONSUMER_CHECK */

static inline int ABTI_pool_remove(ABTI_pool *p_pool, ABT_unit unit,
                                   ABTI_native_thread_id consumer_id)
{
    int abt_errno = ABT_SUCCESS;

    LOG_EVENT_POOL_REMOVE(p_pool, unit, consumer_id);

    abt_errno = ABTI_pool_set_consumer(p_pool, consumer_id);
    ABTI_CHECK_ERROR(abt_errno);

    abt_errno = p_pool->p_remove(ABTI_pool_get_handle(p_pool), unit);
    ABTI_CHECK_ERROR(abt_errno);

fn_exit:
    return abt_errno;

fn_fail:
    HANDLE_ERROR_FUNC_WITH_CODE(abt_errno);
    goto fn_exit;
}

#define ABTI_POOL_REMOVE(p_pool, unit, consumer_id)                            \
    ABTI_pool_remove(p_pool, unit, consumer_id)
#define ABTI_POOL_SET_CONSUMER(p_pool, consumer_id)                            \
    do {                                                                       \
        abt_errno = ABTI_pool_set_consumer(p_pool, consumer_id);               \
        ABTI_CHECK_ERROR(abt_errno);                                           \
    } while (0)

#endif /* ABT_CONFIG_DISABLE_POOL_CONSUMER_CHECK */

static inline ABT_unit ABTI_pool_pop_timedwait(ABTI_pool *p_pool,
                                               double abstime_secs)
{
    ABT_unit unit;

    unit = p_pool->p_pop_timedwait(ABTI_pool_get_handle(p_pool), abstime_secs);
    LOG_EVENT_POOL_POP(p_pool, unit);

    return unit;
}

static inline ABT_unit ABTI_pool_pop(ABTI_pool *p_pool)
{
    ABT_unit unit;

    unit = p_pool->p_pop(ABTI_pool_get_handle(p_pool));
    LOG_EVENT_POOL_POP(p_pool, unit);

    return unit;
}

/* Increase num_scheds to mark the pool as having another scheduler. If the
 * pool is not available, it returns ABT_ERR_INV_POOL_ACCESS.  */
static inline void ABTI_pool_retain(ABTI_pool *p_pool)
{
    ABTD_atomic_fetch_add_int32(&p_pool->num_scheds, 1);
}

/* Decrease the num_scheds to release this pool from a scheduler. Call when
 * the pool is removed from a scheduler or when it stops. */
static inline int32_t ABTI_pool_release(ABTI_pool *p_pool)
{
    ABTI_ASSERT(ABTD_atomic_acquire_load_int32(&p_pool->num_scheds) > 0);
    return ABTD_atomic_fetch_sub_int32(&p_pool->num_scheds, 1) - 1;
}

static inline size_t ABTI_pool_get_size(ABTI_pool *p_pool)
{
    return p_pool->p_get_size(ABTI_pool_get_handle(p_pool));
}

static inline size_t ABTI_pool_get_total_size(ABTI_pool *p_pool)
{
    size_t total_size;
    total_size = ABTI_pool_get_size(p_pool);
    total_size += ABTD_atomic_acquire_load_int32(&p_pool->num_blocked);
    total_size += ABTD_atomic_acquire_load_int32(&p_pool->num_migrations);
    return total_size;
}

#endif /* ABTI_POOL_H_INCLUDED */
