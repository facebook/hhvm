<?hh

<<Foo1, Bar1(), Baz1('blah', varray[1,2])>>
type FBID = int;

<<__StrictType>>
newtype FBID2 as int = FBID;

function wat(@FBID $x) {
  var_dump($x);
}
function make_fbid2(int $id): FBID2 {
  return $id;
}

<<__EntryPoint>>
function main() {
  wat(42);
  wat(make_fbid2(5));
  wat("fail");
  echo "Done\n";
}
