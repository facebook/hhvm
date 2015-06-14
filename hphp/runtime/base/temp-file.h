/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TEMP_FILE_H_
#define incl_HPHP_TEMP_FILE_H_

#include "hphp/runtime/base/plain-file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A temporary read/write file system file for php://temp, it will be deleted
 * from the file system on close.
 */
class TempFile : public PlainFile {
public:
  DECLARE_RESOURCE_ALLOCATION(TempFile);

  explicit TempFile(bool autoDelete = true,
                    const String& wrapper_type = null_string,
                    const String& stream_type = empty_string_ref);
  virtual ~TempFile();

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  // implementing File
  bool open(const String& filename, const String& mode) override;
  bool close() override;

  Object await(uint16_t events, double timeout) override {
    SystemLib::throwExceptionObject(
      "Temporary stream does not support awaiting");
  }

private:
  bool m_autoDelete;
  std::string m_rawName;

  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TEMP_FILE_H_
