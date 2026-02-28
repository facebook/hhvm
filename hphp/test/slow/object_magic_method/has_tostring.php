<?hh

class X {}

class Y extends X {
  function __toString()[] :mixed{ return 'Y'; }
}

function test(X $obj, string $s) :mixed{
  return HH\Lib\Legacy_FIXME\eq($obj, $s);
}

function main() :mixed{
  var_dump(test(new X, "X"));
  var_dump(test(new Y, "Y"));
}


<<__EntryPoint>>
function main_has_tostring() :mixed{
main();
}
