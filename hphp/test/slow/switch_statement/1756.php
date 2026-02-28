<?hh

function f($x) :mixed{
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, "foo", "foo", null, dict[
  "3" => 3,
], dict[
])) {
  case "foo":
    print "foo-0";
  case "3":
    print "3-0";
  case "3":
    print "3-1";
  case "foo":
    print "foo-1";
  case "bar":
    print "bar";
  default:
    print "default";
  }
  print "\n";
}
function g($x) :mixed{
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, 'x', 'x', '0', dict[
  '0' => 0,
], dict[
])) {
  case 'x': print 'x';
 break;
  case '0': print '0';
 break;
  }
}

<<__EntryPoint>>
function main_1756() :mixed{
f("foo");
f("3");
f("bar");
f(null);
f(3);
f(0);
f(0.0);
f(3.0);
f(true);
f(false);
f(vec[]);
f(new stdClass());
g(0);
g(0.0);
}
