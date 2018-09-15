<?hh
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    echo "E_RECOVERABLE_ERROR: $errmsg\n";
  } else {
    return false;
  }
}

<<Foo1, Bar1(), Baz1('blah', array(1,2))>>
type FBID = int;

<<__StrictType>>
newtype FBID2 as int = FBID;

function wat(FBID $x) {
  var_dump($x);
}
function make_fbid2(int $id): FBID2 {
  return $id;
}
function main() {
  wat(42);
  wat(make_fbid2(5));
  wat("fail");
  echo "Done\n";
}


<<__EntryPoint>>
function main_typedef_attr() {
error_reporting(-1);
set_error_handler('handler');

main();
}
