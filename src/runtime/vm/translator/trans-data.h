/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef _TRANS_DATA_H_
#define _TRANS_DATA_H_

#include <util/base.h>
#include <runtime/vm/translator/types.h>

#include <vector>

namespace HPHP {
namespace VM {
namespace Transl {

// Interface for classes maintaining meta-data about translations
// created by the tracelet translator

struct TransRec;

class TransData {
public:
  // Interface to be implemented by subclasses
  virtual ~TransData() {}
  virtual uint32 getNumTrans() const = 0;

  virtual const TransRec* getTransRec(TransID id) const = 0;

  virtual uint64 getTransCounter(TransID id) const = 0;

  virtual const TransIDSet& getTransPreds(TransID transId) const = 0;

  virtual const TransIDSet& getTransSuccs(TransID transId) const = 0;
};

} } }

#endif
