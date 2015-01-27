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

#include <string>

#include <gtest/gtest.h>

#include "hphp/runtime/base/url.h"

using std::string;

namespace HPHP {

TEST(TestUrl, GetServerObject) {
  const char* full_url = "http://facebook.com/foo?x=1";
  const char* expected_object = "/foo?x=1";
  const char* returned_object = URL::getServerObject(full_url);

  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);
  
  full_url = "http://facebook.com/foo/bar?x=1";
  expected_object = "/foo/bar?x=1";
  returned_object = URL::getServerObject(full_url);
  
  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);

  full_url = "https://facebook.com/foo?x=1";
  expected_object = "/foo/bar?x=1";
  returned_object = URL::getServerObject(full_url);
  
  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);

  full_url = "https://facebook.com/foo/bar?x=1";
  expected_object = "/foo/bar?x=1";
  returned_object = URL::getServerObject(full_url);
  
  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);

  full_url = "facebook.com/foo?x=1";
  expected_object = "/foo?x=1";
  returned_object = URL::getServerObject(full_url);
  
  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);

  full_url = "facebook.com/foo/bar?x=1";
  expected_object = "/foo/bar?x=1";
  returned_object = URL::getServerObject(full_url);
  
  ASSERT_TRUE(strcmp(expected_object, returned_object) == 0);

}

}  // namespace HPHP