/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_TEMP_FILE_H_
#define incl_HPHP_TEMP_FILE_H_

#include "hphp/runtime/base/plain_file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A temporary read/write file system file for php://temp, it will be deleted
 * from the file system on close.
 */
class TempFile : public PlainFile {
public:
  DECLARE_OBJECT_ALLOCATION(TempFile);

  explicit TempFile(bool autoDelete = true);
  virtual ~TempFile();

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();

private:
  bool m_autoDelete;
  std::string m_rawName;

  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TEMP_FILE_H_
