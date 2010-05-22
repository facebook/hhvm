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

#include <test/test_ext_url.h>
#include <runtime/ext/ext_url.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtUrl::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_base64_decode);
  RUN_TEST(test_base64_encode);
  RUN_TEST(test_get_headers);
  RUN_TEST(test_get_meta_tags);
  RUN_TEST(test_http_build_query);
  RUN_TEST(test_parse_url);
  RUN_TEST(test_rawurldecode);
  RUN_TEST(test_rawurlencode);
  RUN_TEST(test_urldecode);
  RUN_TEST(test_urlencode);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtUrl::test_base64_decode() {
  VS(f_base64_decode("VGhpcyBpcyBhbiBlbmNvZGVkIHN0cmluZw=="),
     "This is an encoded string");
  VERIFY(same(f_base64_decode("BgAYdjk="),
              String("\006\0\030v9", 5, AttachLiteral)));
  return Count(true);
}

bool TestExtUrl::test_base64_encode() {
  VS(f_base64_encode("This is an encoded string"),
     "VGhpcyBpcyBhbiBlbmNvZGVkIHN0cmluZw==");
  VS(f_base64_encode(String("\006\0\030v9", 5, AttachLiteral)), "BgAYdjk=");
  return Count(true);
}

bool TestExtUrl::test_get_headers() {
  String url = "http://www.example.com";
  Array ret = f_get_headers(url);
  //VS(ret[0], "HTTP/1.1 200 OK");
  VERIFY(ret.size() > 0);
  ret = f_get_headers(url, 1);
  //VS(ret["Connection"], "close");
  VERIFY(!ret["Connection"].toString().empty());
  return Count(true);
}

bool TestExtUrl::test_get_meta_tags() {
  Array ret = f_get_meta_tags("test/test_get_meta_tags.html");
  VS(ret.size(), 4);
  VS(ret["author"], "name");
  VS(ret["keywords"], "php documentation");
  VS(ret["description"], "a php manual");
  VS(ret["geo_position"], "49.33;-86.59");
  return Count(true);
}

bool TestExtUrl::test_http_build_query() {
  {
    Array data = CREATE_MAP4("foo", "bar", "baz", "boom", "cow", "milk",
                             "php", "hypertext processor");
    VS(f_http_build_query(data),
       "foo=bar&baz=boom&cow=milk&php=hypertext+processor");
    VS(f_http_build_query(data, "", "&amp;"),
       "foo=bar&amp;baz=boom&amp;cow=milk&amp;php=hypertext+processor");
  }
  {
    Array data = Array(ArrayInit(6, false).
                       set(0, "foo").
                       set(1, "bar").
                       set(2, "baz").
                       set(3, "boom").
                       set(4, "cow", "milk").
                       set(5, "php", "hypertext processor").
                       create());
    VS(f_http_build_query(data),
       "0=foo&1=bar&2=baz&3=boom&cow=milk&php=hypertext+processor");
    VS(f_http_build_query(data, "myvar_"),
       "myvar_0=foo&myvar_1=bar&myvar_2=baz&myvar_3=boom&cow=milk&"
       "php=hypertext+processor");
  }
  {
    Array data = Array(ArrayInit(4, false).
      set(0, "user",
             CREATE_MAP4("name", "Bob Smith",
                         "age", 47,
                         "sex", "M",
                         "dob", "5/12/1956")).
      set(1, "pastimes",
             CREATE_VECTOR4("golf", "opera", "poker", "rap")).
      set(2, "children",
             CREATE_MAP2("bobby", CREATE_MAP2("age",12,"sex","M"),
                         "sally", CREATE_MAP2("age", 8,"sex","F"))).
      set(3, "CEO").
      create());

    VS(f_http_build_query(data, "flags_"),
       "user%5Bname%5D=Bob+Smith&user%5Bage%5D=47&user%5Bsex%5D=M&"
       "user%5Bdob%5D=5%2F12%2F1956&pastimes%5B0%5D=golf&"
       "pastimes%5B1%5D=opera&pastimes%5B2%5D=poker&"
       "pastimes%5B3%5D=rap&children%5Bbobby%5D%5Bage%5D=12&"
       "children%5Bbobby%5D%5Bsex%5D=M&children%5Bsally%5D%5Bage%5D=8&"
       "children%5Bsally%5D%5Bsex%5D=F&flags_0=CEO");
  }
  {
    Object obj(NEW(c_stdclass)());
    obj->o_set("foo", -1, "bar");
    obj->o_set("baz", -1, "boom");
    VS(f_http_build_query(obj), "foo=bar&baz=boom");
  }
  return Count(true);
}

bool TestExtUrl::test_parse_url() {
  String url = "http://username:password@hostname/path?arg=value#anchor";
  VS(f_print_r(f_parse_url(url), true),
     "Array\n"
     "(\n"
     "    [scheme] => http\n"
     "    [host] => hostname\n"
     "    [user] => username\n"
     "    [pass] => password\n"
     "    [path] => /path\n"
     "    [query] => arg=value\n"
     "    [fragment] => anchor\n"
     ")\n");
  return Count(true);
}

bool TestExtUrl::test_rawurldecode() {
  VS(f_rawurldecode("foo%20bar%40baz"), "foo bar@baz");
  VS(f_rawurldecode("foo+bar%40baz"), "foo+bar@baz");
  return Count(true);
}

bool TestExtUrl::test_rawurlencode() {
  VS(f_rawurlencode("foo bar@baz"), "foo%20bar%40baz");
  return Count(true);
}

bool TestExtUrl::test_urldecode() {
  VS(f_urldecode("foo+bar%40baz"), "foo bar@baz");
  return Count(true);
}

bool TestExtUrl::test_urlencode() {
  VS(f_urlencode("foo bar@baz"), "foo+bar%40baz");
  return Count(true);
}
