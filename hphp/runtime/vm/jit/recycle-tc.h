/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_RUNTIME_VM_JIT_RECYCLE_TC_H_
#define incl_HPHP_RUNTIME_VM_JIT_RECYCLE_TC_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct Func;
struct SrcKey;

namespace jit {

struct TransLoc;
struct SrcRec;

/*
 * TC Reuse Module
 *
 * This module implements garbage collection for the translation cache so that
 * unreachable translations may be overridden by new translations.
 *
 * Unreachable translations are created by either:
 *  (1) Freeing a function through the treadmill
 *  (2) Replacing profiling translations in a SrcRec
 *
 * SrcRecs and prologues are recorded as they are emitted in to the TC so that
 * when their associated function becomes unreachable they may be freed. In the
 * case of profiling translations, these are sometimes freed eagerly when they
 * become unreachable, as they will be erased from their associated SrcRec and
 * are not tracked elsewhere.
 *
 * Function callers and inter-translation jumps are recorded so that they may
 * be smashed when space is reclaimed with the TC.
 *
 * Freed memory is tracked and allocated using the policy defined in DataBlock,
 * and allocation is performed in MCGenerator following the creation of a new
 * translation.
 *
 * Rather than emit new translations directly into freed memory they are written
 * at the end of the TC and then relocated into freed memory. As the space
 * required for a translation will be unknown until it is complete this strategy
 * allows allocation of an appropriately sized block.
 *
 * Currently all allocation and deallocation is done eagerly, therefore the
 * performance of the module is dependent on accurately detecting unreachable
 * functions and translations.
 *
 * This module exports diagnostic data in the form of counts of smashed calls
 * and branches, and recorded functions. Higher level diagnostic data exported
 * by DataBlock may be of more use in tracking TC health. In particular, the
 * number of free bytes and free blocks give a rough measure of fragmentation
 * within the allocator.
 *
 * See DataBlock for details about the allocation strategy and free memory
 * tracking.
 */


/*
 * Record smashed calls in the TC that may need to be re-smashed in the event
 * that a prologue is reused-- additionally any information in ProfData will
 * need to be erased before a translation with a call to a Proflogue is
 * reclaimed.
 *
 * Precondition: Translator::WriteLease().amOwner()
 */
void recordFuncCaller(const Func* func, TCA toSmash, bool immutable,
                      bool profiled, int numArgs);

/*
 * When a function is treadmilled its bytecode may no longer be available,
 * keep a table of associated SrcRecs to be reclaimed as it will be impossible
 * to walk the bytecode stream to search the SrcDB.
 *
 * Precondition: Translator::WriteLease().amOwner()
 */
void recordFuncSrcRec(const Func* func, SrcRec* rec);

/*
 * Record a prologue associated with a function so that it may be reclaimed
 * when the function is treadmilled.
 *
 * Precondition: Translator::WriteLease().amOwner()
 */
void recordFuncPrologue(const Func* func, TransLoc loc);

/*
 * Record a jmp at address toSmash to SrcRec sr.
 *
 * When a translation is reclaimed we remove all annotations from all SrcRecs
 * containing IBs from the translation so that they cannot be inadvertantly
 * smashed in the process of replaceOldTranslations()
 *
 * Precondition: MCGenerator::canWrite()
 */
void recordJump(TCA toSmash, SrcRec* sr);

/*
 * Allows TC space for translation at loc to be reused in future translations.
 *
 * Reclaiming a translation will:
 *  (1) Mark bytes available for reuse in the code-blocks associated with
 *      the translation
 *  (2) Erase any IBs from translation into other SrcRecs
 *  (3) Erase any jump annotations in MCGenerator used to generate optimized
 *      traces
 *  (4) Erase an metadata about smashed calls in the translation from both the
 *      reuse-tc module and the prof-data module
 *
 * The translation _must_ be unreachable before reclaimTranslation() is called,
 * this is generally done by calling reclaimFunction() or performing
 * replaceOldTranslations() on a SrcRec
 */
void reclaimTranslation(TransLoc loc);

/*
 * Reclaim all TC space associated with func
 *
 * Before any space is reclaimed the following actions will be performed:
 *  (1) Smash all prologues
 *  (2) Smash all callers to bind-call unique stubs
 *  (3) Erase all call meta-data for calls into function
 *
 * After all calls and prologues have been smashed any on-going requests will be
 * allowed to complete before TC Space will be reclaimed for:
 *  (1) All prologues
 *  (2) All translations
 *  (3) All anchor translations
 *
 * This function should only be called from Func::destroy() and may access the
 * fullname and ID of the function.
 */
void reclaimFunction(Func* func);

/*
 * Information about the number of bound calls, branches, and tracked functions
 * for use in logging.
 */
int smashedCalls();
int smashedBranches();
int recordedFuncs();

}}

#endif
