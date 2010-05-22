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

#include <test/test_ext_iterator.h>
#include <runtime/ext/ext_iterator.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIterator::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_hphp_recursiveiteratoriterator___construct);
  RUN_TEST(test_hphp_recursiveiteratoriterator_getinneriterator);
  RUN_TEST(test_hphp_recursiveiteratoriterator_current);
  RUN_TEST(test_hphp_recursiveiteratoriterator_key);
  RUN_TEST(test_hphp_recursiveiteratoriterator_next);
  RUN_TEST(test_hphp_recursiveiteratoriterator_rewind);
  RUN_TEST(test_hphp_recursiveiteratoriterator_valid);
  RUN_TEST(test_hphp_directoryiterator___construct);
  RUN_TEST(test_hphp_directoryiterator_getatime);
  RUN_TEST(test_hphp_directoryiterator_getbasename);
  RUN_TEST(test_hphp_directoryiterator_getctime);
  RUN_TEST(test_hphp_directoryiterator_getfilename);
  RUN_TEST(test_hphp_directoryiterator_getgroup);
  RUN_TEST(test_hphp_directoryiterator_getinode);
  RUN_TEST(test_hphp_directoryiterator_getmtime);
  RUN_TEST(test_hphp_directoryiterator_getowner);
  RUN_TEST(test_hphp_directoryiterator_getpathname);
  RUN_TEST(test_hphp_directoryiterator_getperms);
  RUN_TEST(test_hphp_directoryiterator_getsize);
  RUN_TEST(test_hphp_directoryiterator_gettype);
  RUN_TEST(test_hphp_directoryiterator_isdir);
  RUN_TEST(test_hphp_directoryiterator_isdot);
  RUN_TEST(test_hphp_directoryiterator_isexecutable);
  RUN_TEST(test_hphp_directoryiterator_isfile);
  RUN_TEST(test_hphp_directoryiterator_islink);
  RUN_TEST(test_hphp_directoryiterator_isreadable);
  RUN_TEST(test_hphp_directoryiterator_iswritable);
  RUN_TEST(test_hphp_directoryiterator_key);
  RUN_TEST(test_hphp_directoryiterator_next);
  RUN_TEST(test_hphp_directoryiterator_rewind);
  RUN_TEST(test_hphp_directoryiterator_seek);
  RUN_TEST(test_hphp_directoryiterator_current);
  RUN_TEST(test_hphp_directoryiterator___tostring);
  RUN_TEST(test_hphp_directoryiterator_valid);
  RUN_TEST(test_hphp_recursivedirectoryiterator___construct);
  RUN_TEST(test_hphp_recursivedirectoryiterator_key);
  RUN_TEST(test_hphp_recursivedirectoryiterator_next);
  RUN_TEST(test_hphp_recursivedirectoryiterator_rewind);
  RUN_TEST(test_hphp_recursivedirectoryiterator_seek);
  RUN_TEST(test_hphp_recursivedirectoryiterator_current);
  RUN_TEST(test_hphp_recursivedirectoryiterator___tostring);
  RUN_TEST(test_hphp_recursivedirectoryiterator_valid);
  RUN_TEST(test_hphp_recursivedirectoryiterator_haschildren);
  RUN_TEST(test_hphp_recursivedirectoryiterator_getchildren);
  RUN_TEST(test_hphp_recursivedirectoryiterator_getsubpath);
  RUN_TEST(test_hphp_recursivedirectoryiterator_getsubpathname);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIterator::test_hphp_recursiveiteratoriterator___construct() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_getinneriterator() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_current() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_key() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_next() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_rewind() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursiveiteratoriterator_valid() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator___construct() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getatime() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getbasename() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getctime() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getfilename() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getgroup() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getinode() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getmtime() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getowner() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getpathname() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getperms() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_getsize() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_gettype() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_isdir() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_isdot() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_isexecutable() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_isfile() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_islink() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_isreadable() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_iswritable() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_key() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_next() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_rewind() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_seek() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_current() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator___tostring() {
  return Count(true);
}

bool TestExtIterator::test_hphp_directoryiterator_valid() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator___construct() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_key() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_next() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_rewind() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_seek() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_current() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator___tostring() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_valid() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_haschildren() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_getchildren() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_getsubpath() {
  return Count(true);
}

bool TestExtIterator::test_hphp_recursivedirectoryiterator_getsubpathname() {
  return Count(true);
}
