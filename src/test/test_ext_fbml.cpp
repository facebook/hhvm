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

#include <test/test_ext_fbml.h>
#include <cpp/ext/ext_fbml.h>
#include <cpp/ext/ext_output.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtFbml::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_fbml_tag_list_expanded_11);
  RUN_TEST(test_fbml_complex_expand_tag_list_11);
  RUN_TEST(test_fbml_parse_opaque_11);
  RUN_TEST(test_fbml_sanitize_css_11);
  RUN_TEST(test_fbml_sanitize_js_11);
  RUN_TEST(test_fbml_get_tag_name_11);
  RUN_TEST(test_fbml_get_children_11);
  RUN_TEST(test_fbml_get_children_count_11);
  RUN_TEST(test_fbml_get_children_by_name_11);
  RUN_TEST(test_fbml_get_attributes_11);
  RUN_TEST(test_fbml_get_attribute_11);
  RUN_TEST(test_fbml_attr_to_bool_11);
  RUN_TEST(test_fbml_attr_to_color_11);
  RUN_TEST(test_fbml_get_text_11);
  RUN_TEST(test_fbml_precache_11);
  RUN_TEST(test_fbml_batch_precache_11);
  RUN_TEST(test_fbml_render_children_11);
  RUN_TEST(test_fbml_flatten_11);
  RUN_TEST(test_html_profile);
  RUN_TEST(test_fbjsparse);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFbml::test_fbml_tag_list_expanded_11() {
  VERIFY(!f_fbml_tag_list_expanded_11());

  f_fbml_complex_expand_tag_list_11
    (CREATE_VECTOR1("fb:b"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("br"),
     CREATE_VECTOR1("img"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR2("onclick", "onload"),
     CREATE_VECTOR1("rewrite"),
     CREATE_VECTOR1("special"),
     CREATE_MAP1("html", CREATE_VECTOR2("fb:b", "_test")));

  VERIFY(f_fbml_tag_list_expanded_11());
  return Count(true);
}

bool TestExtFbml::test_fbml_complex_expand_tag_list_11() {
  f_fbml_complex_expand_tag_list_11
    (CREATE_VECTOR1("fb:b"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("br"),
     CREATE_VECTOR1("img"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR2("onclick", "onload"),
     CREATE_VECTOR1("rewrite"),
     CREATE_VECTOR1("special"),
     CREATE_MAP1("html", CREATE_VECTOR2("fb:b", "_test")));
  return Count(true);
}

bool TestExtFbml::test_fbml_parse_opaque_11() {
  String fbml = "<b style=\"color:red; colorr:red; "
    "background-image: url(TESTURI)\" onclick=\"a = this.test; with(o){}\">"
    "test</b><!-- some comment -->";
  Variant ret = f_fbml_parse_opaque_11
    (fbml, true, false, false,
     CREATE_MAP3("prefix", "app123", "func", "urltr", "data", "URLDATA"));
  VS(ret["error"], "CSS Error (line 1 char 19): PEUnknownProperty: 'colorr'  PEDeclDropped\n");
  return Count(true);
}

bool TestExtFbml::test_fbml_sanitize_css_11() {
  String css = ".test {color:red; colorr:red; background-image: url(TESTURI)}";
  Variant ret = f_fbml_sanitize_css_11
    (css, false, 2,
     CREATE_MAP3("prefix", "app123", "func", "urltr", "data", "OPAQUE"));
  VS(ret, CREATE_MAP2("sanitized", "app123 .test { color: red; background-image: url(url:OPAQUE=TESTURI); }\n",
                      "error", "CSS Error (line 2 char 26): PEUnknownProperty: 'colorr'  PEDeclDropped\n"));
  return Count(true);
}

bool TestExtFbml::test_fbml_sanitize_js_11() {
  String js = "a = this.test; with(o){}";
  Variant ret = f_fbml_sanitize_js_11(js, 2, CREATE_MAP1("prefix", "app123"));
  VS(ret, CREATE_MAP2("sanitized",
                      "app123a = (ref(this)).test;with (app123o) {}",
                      "error",
                      "JS Error (line 2): \"with\" is not supported.\n"));
  return Count(true);
}

bool TestExtFbml::test_fbml_get_tag_name_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  VS(f_fbml_get_tag_name_11(ret["root"]), "body");
  return Count(true);
}

bool TestExtFbml::test_fbml_get_children_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant children = f_fbml_get_children_11(ret["root"]);
  VS(f_fbml_get_tag_name_11(children[0]), "a");
  VS(f_fbml_get_tag_name_11(children[1]), "b");
  return Count(true);
}

bool TestExtFbml::test_fbml_get_children_count_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  VS(f_fbml_get_children_count_11(ret["root"]), 2);
  return Count(true);
}

bool TestExtFbml::test_fbml_get_children_by_name_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant children = f_fbml_get_children_by_name_11(ret["root"], "a");
  VS(children.toArray().size(), 1);
  VS(f_fbml_get_tag_name_11(children[0]), "a");
  return Count(true);
}

bool TestExtFbml::test_fbml_get_attributes_11() {
  String fbml = "<a href='link' target=_top>text</a>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant children = f_fbml_get_children_11(ret["root"]);
  Object node = children[0].toObject();

  VS(f_fbml_get_attributes_11(node),
     CREATE_MAP2("href", "link", "target", "_top"));
  return Count(true);
}

bool TestExtFbml::test_fbml_get_attribute_11() {
  String fbml = "<a href='link' target=_top>text</a>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant children = f_fbml_get_children_11(ret["root"]);
  Object node = children[0].toObject();

  VS(f_fbml_get_attribute_11(node, "target"), "_top");
  return Count(true);
}

bool TestExtFbml::test_fbml_attr_to_bool_11() {
  VS(f_fbml_attr_to_bool_11("true"),  true);
  VS(f_fbml_attr_to_bool_11("yes"),   true);
  VS(f_fbml_attr_to_bool_11("false"), false);
  VS(f_fbml_attr_to_bool_11("no"),    false);
  return Count(true);
}

bool TestExtFbml::test_fbml_attr_to_color_11() {
  VS(f_fbml_attr_to_color_11("red"),  "red");
  VS(f_fbml_attr_to_color_11("redd"), null);
  return Count(true);
}

bool TestExtFbml::test_fbml_get_text_11() {
  String fbml = "<a>text</a>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant c = f_fbml_get_children_11(ret["root"]);
  Variant d = f_fbml_get_children_11(c[0]);
  VS(f_fbml_get_text_11(d[0]), "text");
  return Count(true);
}

bool TestExtFbml::test_fbml_precache_11() {
  f_fbml_complex_expand_tag_list_11
    (CREATE_VECTOR1("fb:b"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("br"),
     CREATE_VECTOR1("a"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR2("onclick", "onload"),
     CREATE_VECTOR1("rewrite"),
     CREATE_VECTOR1("special"),
     CREATE_MAP1("html", CREATE_VECTOR2("fb:b", "_test")));

  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  f_ob_start();
  f_fbml_precache_11(ret["root"], "tag:", "precache");
  //VS(f_ob_get_contents(), "tag:a");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtFbml::test_fbml_batch_precache_11() {
  f_fbml_complex_expand_tag_list_11
    (CREATE_VECTOR1("fb:b"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("br"),
     CREATE_VECTOR1("a"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR1("style"),
     CREATE_VECTOR2("onclick", "onload"),
     CREATE_VECTOR1("rewrite"),
     CREATE_VECTOR1("special"),
     CREATE_MAP1("html", CREATE_VECTOR2("fb:b", "_test")));

  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant precachable = f_fbml_batch_precache_11(ret["root"]);
  Array nodes = precachable.toArray();
  for (ArrayIter iter(nodes); iter; ++iter) {
    VS(iter.first(), "a");
    Array arr = iter.second().toArray();
    for (ArrayIter iterNode(arr); iterNode; ++iterNode) {
      VS(f_fbml_get_tag_name_11(iterNode.second()), "a");
    }
  }
  return Count(true);
}

bool TestExtFbml::test_fbml_render_children_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  VS(f_fbml_render_children_11(ret["root"], "data", "", ""),
     "<a>link</a><b>text</b>");
  return Count(true);
}

bool TestExtFbml::test_fbml_flatten_11() {
  String fbml = "<a>link</a><b>text</b>";
  Variant ret = f_fbml_parse_opaque_11(fbml, true, false);
  Variant children = f_fbml_get_children_11(ret["root"]);
  VS(f_fbml_flatten_11(children[1]), "<b>text</b>");
  return Count(true);
}

bool TestExtFbml::test_html_profile() {
  VS(f_html_profile("<html><head><style>"
                    " .useless_css { font-size: 8px;} "
                    " .useful_css  { font-color: red;} "
                    "</style></head>"
                    "<body class='useful_css'><script>"
                    " function useless_func() {}"
                    " function useful_func() {}"
                    " useful_func();"
                    "</script>"
                    "<!-- HTML Comments -->"
                    "</body></html>"),
     "{ \"sizes\": {\"css\":{\"original\":67,\"pickled\":50,\"trimmed\":17}\n"
     ",\"html\":{\"original\":112,\"pickled\":77,\"trimmed\":77}\n"
     ",\"js\":{\"original\":68,\"pickled\":69,\"trimmed\":41}\n"
     ",\"total\":{\"original\":247,\"pickled\":196,\"trimmed\":135}\n"
     "}\n"
     ",\n"
     "\"deps\": {\"js_kept\":{\"_\":[\"useful_func\"]}\n"
     ",\"css_kept\":{\"_\":[\".useful_css\"]}\n"
     ",\"css_trimmed\":{\"_\":[\".useless_css\"]}\n"
     ",\"js_trimmed\":{\"_\":[\"useless_func\"]}\n"
     "}\n"
     ",\n"
     "\"errs\": {\"css\":[[\"PEUnknownProperty: 'font-color'  PEDeclDropped\",\"\",\"1\",\"61\"]]}\n"
     "\n"
     "}\n"
);
  return Count(true);
}

bool TestExtFbml::test_fbjsparse() {
  try {
    f_fbjsparse("");
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}
