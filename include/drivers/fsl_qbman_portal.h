/* Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FSL_QBMAN_PORTAL_H
#define _FSL_QBMAN_PORTAL_H

#include <drivers/fsl_qbman_base.h>

/* Create and destroy a functional object representing the given QBMan portal
 * descriptor. */
struct qbman_swp *qbman_swp_init(const struct qbman_swp_desc *);
void qbman_swp_finish(struct qbman_swp *);

/* Returns the descriptor for this portal. */
const struct qbman_swp_desc *qbman_swp_get_desc(struct qbman_swp *);

	/**************/
	/* Interrupts */
	/**************/

/* See the QBMan driver API documentation for details on the interrupt
 * mechanisms. */
#define QBMAN_SWP_INTERRUPT_EQRI ((uint32_t)0x00000001)
#define QBMAN_SWP_INTERRUPT_EQDI ((uint32_t)0x00000002)
#define QBMAN_SWP_INTERRUPT_DQRI ((uint32_t)0x00000004)
#define QBMAN_SWP_INTERRUPT_RCRI ((uint32_t)0x00000008)
#define QBMAN_SWP_INTERRUPT_RCDI ((uint32_t)0x00000010)
#define QBMAN_SWP_INTERRUPT_VDCI ((uint32_t)0x00000020)
uint32_t qbman_swp_interrupt_get_vanish(struct qbman_swp *);
void qbman_swp_interrupt_set_vanish(struct qbman_swp *, uint32_t mask);
uint32_t qbman_swp_interrupt_read_status(struct qbman_swp *);
void qbman_swp_interrupt_clear_status(struct qbman_swp *, uint32_t mask);
uint32_t qbman_swp_interrupt_get_trigger(struct qbman_swp *);
void qbman_swp_interrupt_set_trigger(struct qbman_swp *, uint32_t mask);
int qbman_swp_interrupt_get_inhibit(struct qbman_swp *);
void qbman_swp_interrupt_set_inhibit(struct qbman_swp *, int inhibit);

	/************/
	/* Dequeues */
	/************/

/* See the QBMan driver API documentation for details on the enqueue
 * mechanisms. */
struct qbman_dq_entry {
	uint32_t dont_manipulate_directly[16];
};

/* A DQRI interrupt can be generated when there are dequeue results on the
 * portal's DQRR (this mechanism does not deal with "pull" dequeues to
 * user-supplied 'storage' addresses). There are two parameters to this
 * interrupt source, one is a threshold and the other is a timeout. The
 * interrupt will fire if either the fill-level of the ring exceeds 'thresh', or
 * if the ring has been non-empty for been longer than 'timeout' nanoseconds.
 * For timeout, an approximation to the desired nanosecond-granularity value is
 * made, so there are get and set APIs to allow the user to see what actual
 * timeout is set (compared to the timeout that was requested). */
int qbman_swp_dequeue_thresh(struct qbman_swp *, unsigned int thresh);
int qbman_swp_dequeue_set_timeout(struct qbman_swp *, unsigned int timeout);
int qbman_swp_dequeue_get_timeout(struct qbman_swp *, unsigned int *timeout);

/* ------------------- */
/* Push-mode dequeuing */
/* ------------------- */

/* The user of a portal can enable and disable push-mode dequeuing of up to 16
 * channels independently. It does not specify this toggling by channel IDs, but
 * rather by specifing the index (from 0 to 15) that has been mapped to the
 * desired channel. */
void qbman_swp_push_get(struct qbman_swp *, uint8_t channel_idx, int *enabled);
void qbman_swp_push_set(struct qbman_swp *, uint8_t channel_idx, int enable);

/* ------------------- */
/* Pull-mode dequeuing */
/* ------------------- */

struct qbman_pull_desc {
	uint32_t dont_manipulate_directly[6];
};

enum qbman_pull_type_e {
	/* dequeue with priority precedence, respect intra-class scheduling */
	qbman_pull_type_prio,
	/* dequeue with active FQ precedence, respect ICS */
	qbman_pull_type_active,
	/* dequeue with active FQ precedence, no ICS */
	qbman_pull_type_active_noics
};

/* Clear the contents of a descriptor to default/starting state. */
void qbman_pull_desc_clear(struct qbman_pull_desc *);
/* If not called, or if called with 'storage' as NULL, the result pull dequeues
 * will produce results to DQRR. If 'storage' is non-NULL, then results are
 * produced to the given memory location (using the physical/DMA address which
 * the caller provides in 'storage_phys'), and 'stash' controls whether or not
 * those writes to main-memory express a cache-warming attribute. */
void qbman_pull_desc_set_storage(struct qbman_pull_desc *,
				 struct qbman_dq_entry *storage,
				 dma_addr_t storage_phys,
				 int stash);
/* numframes must be between 1 and 16, inclusive */
void qbman_pull_desc_set_numframes(struct qbman_pull_desc *, uint8_t numframes);
/* token is the value that shows up in the dequeue results that can be used to
 * detect when the results have been published, and is not really used when
 * dequeue results go to DQRR. The easiest technique is to zero result "storage"
 * before issuing a pull dequeue, and use any non-zero 'token' value. */
void qbman_pull_desc_set_token(struct qbman_pull_desc *, uint8_t token);
/* Exactly one of the following descriptor "actions" should be set. (Calling any
 * one of these will replace the effect of any prior call to one of these.)
 * - pull dequeue from the given frame queue (FQ)
 * - pull dequeue from any FQ in the given work queue (WQ)
 * - pull dequeue from any FQ in any WQ in the given channel
 */
void qbman_pull_desc_set_fq(struct qbman_pull_desc *, uint32_t fqid);
void qbman_pull_desc_set_wq(struct qbman_pull_desc *, uint32_t wqid,
			    enum qbman_pull_type_e dct);
void qbman_pull_desc_set_channel(struct qbman_pull_desc *, uint32_t chid,
				 enum qbman_pull_type_e dct);

/* Issue the pull dequeue command */
int qbman_swp_pull(struct qbman_swp *, struct qbman_pull_desc *);

/* -------------------------------- */
/* Polling DQRR for dequeue results */
/* -------------------------------- */

/* NULL return if there are no unconsumed DQRR entries. Returns a DQRR entry
 * only once, so repeated calls can return a sequence of DQRR entries, without
 * requiring they be consumed immediately or in any particular order. */
const struct qbman_dq_entry *qbman_swp_dqrr_next(struct qbman_swp *);
/* Consume DQRR entries previously returned from qbman_swp_dqrr_next(). */
void qbman_swp_dqrr_consume(struct qbman_swp *, const struct qbman_dq_entry *);

/* ------------------------------------------------- */
/* Polling user-provided storage for dequeue results */
/* ------------------------------------------------- */

/* Only used for user-provided storage of dequeue results, not DQRR. Prior to
 * being used, the storage must set "oldtoken", so that the driver notices when
 * hardware has filled it in with results using a "newtoken". NB, for efficiency
 * purposes, the driver will perform any required endianness conversion to
 * ensure that the user's dequeue result storage is in host-endian format
 * (whether or not that is the same as the little-endian format that hardware
 * DMA'd to the user's storage). As such, once the user has called
 * qbman_dq_entry_has_newtoken() and been returned a valid dequeue result, they
 * should not call it again on the same memory location (except of course if
 * another dequeue command has been executed to produce a new result to that
 * location).
 */
void qbman_dq_entry_set_oldtoken(struct qbman_dq_entry *,
				 unsigned int num_entries,
				 uint8_t oldtoken);
int qbman_dq_entry_has_newtoken(struct qbman_swp *,
				const struct qbman_dq_entry *,
				uint8_t newtoken);

/* -------------------------------------------------------- */
/* Parsing dequeue entries (DQRR and user-provided storage) */
/* -------------------------------------------------------- */

/* DQRR entries may contain non-dequeue results, ie. notifications */
int qbman_dq_entry_is_DQ(const struct qbman_dq_entry *);
/* Recognise different notification types, only required if the user allows for
 * these to occur, and cares about them when they do. */
int qbman_dq_entry_is_FQDAN(const struct qbman_dq_entry *);
				/* FQ Data Availability */
int qbman_dq_entry_is_CDAN(const struct qbman_dq_entry *);
				/* Channel Data Availability */
int qbman_dq_entry_is_CSCN(const struct qbman_dq_entry *);
				/* Congestion State Change */
int qbman_dq_entry_is_BPSCN(const struct qbman_dq_entry *);
				/* Buffer Pool State Change */
int qbman_dq_entry_is_CGCU(const struct qbman_dq_entry *);
				/* Congestion Group Count Update */
/* Frame queue state change notifications; (FQDAN in theory counts too as it
 * leaves a FQ parked, but it is primarily a data availability notification) */
int qbman_dq_entry_is_FQRN(const struct qbman_dq_entry *); /* Retirement */
int qbman_dq_entry_is_FQRNI(const struct qbman_dq_entry *);
				/* Retirement Immediate */
int qbman_dq_entry_is_FQPN(const struct qbman_dq_entry *); /* Park */

/* Parsing frame dequeue results (qbman_dq_entry_is_DQ() must be TRUE) */
#define QBMAN_DQ_STAT_FQEMPTY       0x80
#define QBMAN_DQ_STAT_HELDACTIVE    0x40
#define QBMAN_DQ_STAT_FORCEELIGIBLE 0x20
#define QBMAN_DQ_STAT_VALIDFRAME    0x10
#define QBMAN_DQ_STAT_ODPVALID      0x04
#define QBMAN_DQ_STAT_VOLATILE      0x02
#define QBMAN_DQ_STAT_EXPIRED       0x01
uint32_t qbman_dq_entry_DQ_flags(const struct qbman_dq_entry *);
static inline int qbman_dq_entry_DQ_is_pull(const struct qbman_dq_entry *dq)
{
	return (int)(qbman_dq_entry_DQ_flags(dq) & QBMAN_DQ_STAT_VOLATILE);
}
static inline int qbman_dq_entry_DQ_is_pull_complete(
					const struct qbman_dq_entry *dq)
{
	return (int)(qbman_dq_entry_DQ_flags(dq) & QBMAN_DQ_STAT_EXPIRED);
}
/* seqnum/odpid are valid only if VALIDFRAME and ODPVALID flags are TRUE */
uint16_t qbman_dq_entry_DQ_seqnum(const struct qbman_dq_entry *);
uint16_t qbman_dq_entry_DQ_odpid(const struct qbman_dq_entry *);
uint32_t qbman_dq_entry_DQ_fqid(const struct qbman_dq_entry *);
uint32_t qbman_dq_entry_DQ_byte_count(const struct qbman_dq_entry *);
uint32_t qbman_dq_entry_DQ_frame_count(const struct qbman_dq_entry *);
uint32_t qbman_dq_entry_DQ_fqd_ctx_hi(const struct qbman_dq_entry *);
uint32_t qbman_dq_entry_DQ_fqd_ctx_lo(const struct qbman_dq_entry *);
const struct qbman_fd *qbman_dq_entry_DQ_fd(const struct qbman_dq_entry *);
							/* Frame Descriptor */

	/************/
	/* Enqueues */
	/************/

struct qbman_eq_desc {
	uint32_t dont_manipulate_directly[8];
};

struct qbman_eq_response {
	uint32_t dont_manipulate_directly[16];
};

/* Clear the contents of a descriptor to default/starting state. */
void qbman_eq_desc_clear(struct qbman_eq_desc *);
/* Exactly one of the following descriptor "actions" should be set. (Calling
 * any one of these will replace the effect of any prior call to one of these.)
 * - enqueue without order-restoration
 * - enqueue with order-restoration
 * - fill a hole in the order-restoration sequence, without any enqueue
 * - advance NESN (Next Expected Sequence Number), without any enqueue
 * 'respond_success' indicates whether an enqueue response should be DMA'd
 * after success (otherwise a response is DMA'd only after failure).
 * 'incomplete' indicates that other fragments of the same 'seqnum' are yet to
 * be enqueued.
 */
void qbman_eq_desc_set_no_orp(struct qbman_eq_desc *, int respond_success);
void qbman_eq_desc_set_orp(struct qbman_eq_desc *, int respond_success,
			   uint32_t orp_id, uint32_t seqnum, int incomplete);
void qbman_eq_desc_set_orp_hole(struct qbman_eq_desc *, uint32_t orp_id,
				uint32_t seqnum);
void qbman_eq_desc_set_orp_nesn(struct qbman_eq_desc *, uint32_t orp_id,
				uint32_t seqnum);
/* In the case where an enqueue response is DMA'd, this determines where that
 * response should go. (The physical/DMA address is given for hardware's
 * benefit, but software should interpret it as a "struct qbman_eq_response"
 * data structure.) 'stash' controls whether or not the write to main-memory
 * expresses a cache-warming attribute. */
void qbman_eq_desc_set_response(struct qbman_eq_desc *,
				dma_addr_t storage_phys,
				int stash);
/* token is the value that shows up in an enqueue response that can be used to
 * detect when the results have been published. The easiest technique is to zero
 * result "storage" before issuing an enqueue, and use any non-zero 'token'
 * value. */
void qbman_eq_desc_set_token(struct qbman_eq_desc *, uint8_t token);
/* Exactly one of the following descriptor "targets" should be set. (Calling any
 * one of these will replace the effect of any prior call to one of these.)
 * - enqueue to a frame queue
 * - enqueue to a queuing destination
 * Note, that none of these will have any affect if the "action" type has been
 * set to "orp_hole" or "orp_nesn".
 */
void qbman_eq_desc_set_fq(struct qbman_eq_desc *, uint32_t fqid);
void qbman_eq_desc_set_qd(struct qbman_eq_desc *, uint32_t qdid,
			  uint32_t qd_bin, uint32_t qd_prio);
/* Determines whether or not the portal's EQDI interrupt source should be
 * asserted after the enqueue command is completed. */
void qbman_eq_desc_set_eqdi(struct qbman_eq_desc *, int enable);
/* Determines whether or not a portal DQRR entry should be consumed once the
 * enqueue command is completed. (And if so, and the DQRR entry corresponds to a
 * held-active (order-preserving) FQ, whether the FQ should be parked instead of
 * being rescheduled.) */
void qbman_eq_desc_set_dca(struct qbman_eq_desc *, int enable,
				uint32_t dqrr_idx, int park);

/* Issue an enqueue command. ('fd' should only be NULL if the "action" of the
 * descriptor is "orp_hole" or "orp_nesn".) */
int qbman_swp_enqueue(struct qbman_swp *, const struct qbman_eq_desc *,
		      const struct qbman_fd *fd);

/* An EQRI interrupt can be generated when the fill-level of EQCR falls below
 * the 'thresh' value set here. Setting thresh==0 (the default) disables. */
int qbman_swp_enqueue_thresh(struct qbman_swp *, unsigned int thresh);

	/*******************/
	/* Buffer releases */
	/*******************/

struct qbman_release_desc {
	uint32_t dont_manipulate_directly[1];
};

/* Clear the contents of a descriptor to default/starting state. */
void qbman_release_desc_clear(struct qbman_release_desc *);
/* Set the ID of the buffer pool to release to */
void qbman_release_desc_set_bpid(struct qbman_release_desc *, uint32_t bpid);
/* Determines whether or not the portal's RCDI interrupt source should be
 * asserted after the release command is completed. */
void qbman_release_desc_set_rcdi(struct qbman_release_desc *, int enable);

/* Issue a release command. 'num_buffers' must be less than 8. */
int qbman_swp_release(struct qbman_swp *, const struct qbman_release_desc *,
		      const uint64_t *buffers, unsigned int num_buffers);

/* An RCRI interrupt can be generated when the fill-level of RCR falls below
 * the 'thresh' value set here. Setting thresh==0 (the default) disables. */
int qbman_swp_release_thresh(struct qbman_swp *, unsigned int thresh);

	/*******************/
	/* Buffer acquires */
	/*******************/

int qbman_swp_acquire(struct qbman_swp *, uint32_t bpid, uint64_t *buffers,
		      unsigned int num_buffers);

	/*****************/
	/* FQ management */
	/*****************/

/* There are a couple of different ways that a FQ can end up parked state,
 * This schedules it. */
int qbman_swp_fq_schedule(struct qbman_swp *, uint32_t fqid);

/* Force eligible will force a tentatively-scheduled FQ to be fully-scheduled
 * and thus be available for selection by any channel-dequeuing behaviour (push
 * or pull). If the FQ is subsequently "dequeued" from the channel and is still
 * empty at the time this happens, the resulting dq_entry will have no FD.
 * (qbman_dq_entry_DQ_fd() will return NULL.) */
int qbman_swp_fq_force(struct qbman_swp *, uint32_t fqid);

/* These functions change the FQ flow-control stuff between XON/XOFF. (The
 * default is XON.) This setting doesn't affect enqueues to the FQ, just
 * dequeues. XOFF FQs will remain in the tenatively-scheduled state, even when
 * non-empty, meaning they won't be selected for scheduled dequeuing. If a FQ is
 * changed to XOFF after it had already become truly-scheduled to a channel, and
 * a pull dequeue of that channel occurs that selects that FQ for dequeuing,
 * then the resulting dq_entry will have no FD. (qbman_dq_entry_DQ_fd() will
 * return NULL.) */
int qbman_swp_fq_xon(struct qbman_swp *, uint32_t fqid);
int qbman_swp_fq_xoff(struct qbman_swp *, uint32_t fqid);

	/**********************/
	/* Channel management */
	/**********************/

/* If the user has been allocated a channel object that is going to generate
 * CDANs to another channel, then these functions will be necessary.
 * CDAN-enabled channels only generate a single CDAN notification, after which
 * it they need to be reenabled before they'll generate another. (The idea is
 * that pull dequeuing will occur in reaction to the CDAN, followed by a
 * reenable step.) Each function generates a distinct command to hardware, so a
 * combination function is provided if the user wishes to modify the "context"
 * (which shows up in each CDAN message) each time they reenable, as a single
 * command to hardware. */
int qbman_swp_CDAN_set_context(struct qbman_swp *, uint16_t channelid,
				uint32_t ctx_hi, uint32_t ctx_lo);
int qbman_swp_CDAN_enable(struct qbman_swp *, uint16_t channelid);
int qbman_swp_CDAN_set_context_enable(struct qbman_swp *, uint16_t channelid,
				      uint32_t ctx_hi, uint32_t ctx_lo);

#endif /* !_FSL_QBMAN_PORTAL_H */
