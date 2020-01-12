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
#ifndef incl_HPHP_TRANS_REC_H_
#define incl_HPHP_TRANS_REC_H_

#include "hphp/util/sha1.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

/*
 * Used to maintain a mapping from the bytecode to its corresponding x86.
 */
struct TransBCMapping {
  SHA1   sha1;
  Offset bcStart;
  TCA    aStart;
  TCA    acoldStart;
  TCA    afrozenStart;
};

/*
 * A record with various information about a translation.
 */
struct TransRec {
  struct Block {
    SHA1 sha1;
    Offset bcStart;
    Offset bcPast;
  };

  std::vector<std::string>
                         guards;
  std::vector<Block>     blocks;
  std::vector<TransBCMapping>
                         bcMapping;
  Annotations            annotations;
  std::string            funcName;
  SrcKey                 src;
  SHA1                   sha1;
  TCA                    aStart;
  TCA                    acoldStart;
  TCA                    afrozenStart;
  uint32_t               aLen;
  uint32_t               acoldLen;
  uint32_t               afrozenLen;
  Offset                 bcStart;
  TransID                id{kInvalidTransID};
  TransKind              kind;
  bool                   hasLoop;

  TransRec() {}

  TransRec(SrcKey                      s,
           TransID                     transID,
           TransKind                   _kind,
           TCA                         _aStart,
           uint32_t                    _aLen,
           TCA                         _acoldStart,
           uint32_t                    _acoldLen,
           TCA                         _afrozenStart,
           uint32_t                    _afrozenLen,
           RegionDescPtr               region = RegionDescPtr(),
           std::vector<TransBCMapping> _bcMapping =
             std::vector<TransBCMapping>(),
           Annotations&&               _annotations =
             Annotations(),
           bool                        _hasLoop = false);

  bool isValid() const { return id != kInvalidTransID; }
  bool isConsistent() const;
  bool contains(TCA tca) const;
  std::string print() const;
  Offset bcPast() const;
  void optimizeForMemory();

private:
  struct SavedAnnotation {
    std::string   fileName;
    uint64_t      offset;
    uint32_t      length;
  };
  SavedAnnotation writeAnnotation(const Annotation& annotation,
                                  bool compress = true);
};

} }

#endif
