/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "compiler/analysis/data_flow.h"

using namespace HPHP;
using std::pair;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

const char *DataFlow::GetName(int i) {
#define DECLARE_DF_NAME(x,v) #x
  static const char *names[] = {
    DECLARE_DATA_FLOW(DECLARE_DF_NAME)
  };
  if (i >= 0 && i < NumBVs) return names[i];
  return "Unknown";
}

int DataFlow::GetInit(int i) {
#define DECLARE_DF_INIT(x,v) v
  static int inits[] = {
    DECLARE_DATA_FLOW(DECLARE_DF_INIT)
  };
  if (i >= 0 && i < NumBVs) return inits[i];
  return 0;
}

void DataFlow::ComputeAvailable(const ControlFlowGraph &g) {
  int num = g.getNumBlocks();
  bool changed;
  BitOps::Bits *tmp1 = g.getTempBits(0);
  bool hasAltered = g.rowExists(Altered);
  size_t width = g.bitWidth();
  bool firstTime = true;

  do {
    changed = false;

    for (int i = 1; i <= num; i++) {
      ControlBlock *b = g.getDfBlock(i);
      std::pair<in_edge_iterator, in_edge_iterator> vi =
        in_edges(b, g);
      BitOps::Bits *ain = b->getRow(AvailIn);

      if (vi.first != vi.second) {
        ControlBlock *p = source(*vi.first, g);
        if (++vi.first != vi.second) {
          if (!changed) BitOps::bit_copy(width, tmp1, ain);
          BitOps::bit_and(width, ain,
                          p->getRow(AvailOut),
                          source(*vi.first, g)->getRow(AvailOut));
          while (++vi.first != vi.second) {
            p = source(*vi.first, g);
            BitOps::bit_and(width, ain, ain, p->getRow(AvailOut));
          }
          if (!changed) changed = !BitOps::bit_equal(width, tmp1, ain);
        } else {
          if (!changed) {
            changed = !BitOps::bit_equal(width, ain, p->getRow(AvailOut));
          }
          BitOps::bit_copy(width, ain, p->getRow(AvailOut));
        }
      } else if (firstTime) {
        // available defaults to all 1s
        // if there are no preds, set to all 0s
        BitOps::set(width, ain, 0);
      }

      BitOps::Bits *aout = b->getRow(AvailOut);
      if (!changed) BitOps::bit_copy(width, tmp1, aout);
      BitOps::Bits *avl = b->getRow(Available);
      if (hasAltered) {
        BitOps::Bits *alt = b->getRow(Altered);
        BitOps::bit_andc_or(width, aout, ain, alt, avl);
      } else {
        BitOps::bit_or(width, aout, ain, avl);
      }
      if (!changed) changed = !BitOps::bit_equal(width, tmp1, aout);
    }
    firstTime = false;
  } while (changed);
}

void DataFlow::ComputeAnticipated(const ControlFlowGraph &g) {
  int num = g.getNumBlocks();
  bool changed;
  BitOps::Bits *tmp1 = g.getTempBits(0);
  bool hasAltered = g.rowExists(Altered);
  size_t width = g.bitWidth();
  bool firstTime = true;

  do {
    changed = false;

    for (int i = num; i ; i--) {
      ControlBlock *b = g.getDfBlock(i);
      std::pair<out_edge_iterator, out_edge_iterator> vi =
        out_edges(b, g);
      BitOps::Bits *aout = b->getRow(AntOut);

      if (vi.first != vi.second) {
        ControlBlock *s = target(*vi.first, g);
        if (++vi.first != vi.second) {
          if (!changed) BitOps::bit_copy(width, tmp1, aout);
          BitOps::bit_and(width, aout,
                          s->getRow(AntIn),
                          target(*vi.first, g)->getRow(AntIn));
          while (++vi.first != vi.second) {
            s = target(*vi.first, g);
            BitOps::bit_and(width, aout, aout, s->getRow(AntIn));
          }
          if (!changed) changed = !BitOps::bit_equal(width, tmp1, aout);
        } else {
          if (!changed) {
            changed = !BitOps::bit_equal(width, aout, s->getRow(AntIn));
          }
          BitOps::bit_copy(width, aout, s->getRow(AntIn));
        }
      } else if (firstTime) {
        // anticipated defaults to all 1s
        // if there are no succs, set to all 0s
        BitOps::set(width, aout, 0);
      }

      BitOps::Bits *ain = b->getRow(AntIn);
      if (!changed) BitOps::bit_copy(width, tmp1, ain);
      BitOps::Bits *ant = b->getRow(Anticipated);
      if (hasAltered) {
        BitOps::Bits *alt = b->getRow(Altered);
        BitOps::bit_andc_or(width, ain, aout, alt, ant);
      } else {
        BitOps::bit_or(width, ain, aout, ant);
      }
      if (!changed) changed = !BitOps::bit_equal(width, tmp1, ain);
    }
  } while (changed);
}
