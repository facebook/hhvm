/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_DCE_H_
#define incl_HPHP_DCE_H_

namespace HPHP { namespace jit {

struct IRUnit;
struct IRInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * Perform very minimal DCE required to preserve the IR invariant that there
 * are no unreachable blocks in the unit.  All other IR invariants must hold
 * before running this function.
 */
void mandatoryDCE(IRUnit&);

/*
 * Perform a full DCE pass, which implies the mandatoryDCE as well.
 *
 * All IR invariants must hold before running this pass, except the one
 * restored by mandatoryDCE, which we allow passes to violate and rely on a
 * fullDCE run to fix.
 */
void fullDCE(IRUnit&);

/*
 * Converts an instruction that operates on a FramePtr in an inlined
 * function to one that operates on a parent FramePtr along with a
 * static offset. This is only allowed for instructions which only
 * need the FramePtr as a base for some other address calculation (and
 * does not actually access the ActRec). The parent FramePtr must not
 * be a resumed frame, and the instruction cannot raise an
 * error. Useful for eliding DefInlineFP.
 */
void convertToUseParentFrameWithOffset(IRUnit& unit, IRInstruction& inst,
                                       SSATmp* parentFp);

/*
 * Converts an InlineReturn instruction to a noop instruction that still models
 * the memory effects of InlineReturn to ensure that stores from the callee are
 * not pushed into the caller, and to hopefully prevent some stores from
 * occuring at all.
 *
 * Precondition: inst is InlineReturn
 * Postcondition: inst is InlineReturnNoFrame
 */
void convertToInlineReturnNoFrame(IRUnit& unit, IRInstruction& inst);

/*
 * Given a SSATmp representing a FramePtr which comes from a DefLabel
 * instruction, chase the definitions to the first non-DefLabel instruction it
 * can find. This assumes that all inputs to the DefLabel are equivalent.
 *
 * Precondition: fp->inst()->is(DefLabel) && fp->is(TFramePtr)
 */
IRInstruction* resolveFpDefLabel(const SSATmp* fp);

/*
 * Checks if the given FramePtr points to a resumed frame, looking
 * through chains of DefLabels if necessary.
 */
bool fpIsResumed(const SSATmp* fp);

/*
 * Checks if the immediate parent of the given FramePtr points to a
 * resumed frame, looking through chains of DefLabels if
 * necessary. Note: if the given FramePtr has no parent (because its
 * not an inlined frame), this returns false, even if the given
 * FramePtr itself is a resumed frame.
 */
bool parentFpIsResumed(const SSATmp* fp);

//////////////////////////////////////////////////////////////////////

}}

#endif
