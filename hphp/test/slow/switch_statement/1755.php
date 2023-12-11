<?hh

class Evil {
  public function __toString()[] :mixed{ return "Evil"; }
}
function f_1($x) :mixed{
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, "123", "0", "0", dict[
  "123" => 123,
  "4abc" => 4,
  "0" => 0,
], dict[
])) {
  case "123": print '"123"' . "";
 break;
  case "4abc": print '"4abc"' . "";
 break;
  case "0": print '"0"' . "";
 break;
  case "": print '""' . "";
 break;
  case "Evil": print '"Evil"' . "";
 break;
  default: print "default";
 break;
  }
}
function f_2($x) :mixed{
  var_dump($x);
 print "  goes to:";
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, "foo", "foo", "0", dict[
  "1" => 1,
  "2.0" => 2,
  "2ab" => 2,
  "0" => 0,
], dict[
  "3.212" => 3.212,
])) {
  case "foo": print "foo";
 break;
  case "1": print "1";
 break;
  case "2.0": print "2.0";
 break;
  case "2ab": print "2ab";
 break;
  case "3.212": print "3.212";
 break;
  case "0": print "0";
 break;
  case "": print "{empty str}";
 break;
  default: print "default";
 break;
  }
}
function g_2($x) :mixed{
  var_dump($x);
 print "  goes to:";
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, null, "", "", dict[
  "0" => 0,
], dict[
])) {
  case "": print "{empty str}";
 break;
  case "0": print "0";
 break;
  default: print "default";
 break;
  }
}
function h_2($x) :mixed{
  var_dump($x);
 print "  goes to:";
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, "3.0", null, null, dict[
  "3.0" => 3,
  "3.0abc" => 3,
  "3" => 3,
], dict[
])) {
  case "3.0": print "3.0";
 break;
  case "3.0abc": print "3.0abc";
 break;
  case "3": print "3";
 break;
  default: print "";
  }
}
function f_3($x) :mixed{
  switch ($x) {
  case "bar": print "bar";
  case "foo": print "foo";
  case "baz": print "baz";
  default: print "default";
  }
}

<<__EntryPoint>>
function main_1755() :mixed{
f_1("");
f_1(null);
f_1(false);
f_1("0");
f_1("0eab");
f_1("0.0");
f_1(0.0);
f_1(0);
f_1(true);
f_1(false);
f_1("4abc");
f_1(4);
f_1("4.0");
f_1(new Evil());
f_2(1);
f_2(2);
f_2(2.0);
f_2(true);
f_2(false);
f_2(null);
f_2(vec[]);
f_2(3.21200);
g_2(0);
g_2(null);
g_2(false);
g_2(true);
h_2("3");
h_2("3abc");
h_2("3a");
h_2(3);
h_2(3.0);
f_3("foo");
f_3("bar");
f_3("baz");
f_3("def");
}
