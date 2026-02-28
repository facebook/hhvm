<?hh

<<Foo1, Bar1(), Baz1('blah', vec[1,2])>>
type FBID = int;

<<__StrictType>>
newtype FBID2 as int = FBID;

function wat(<<__Soft>> FBID $x) :mixed{
  var_dump($x);
}
function make_fbid2(int $id): FBID2 {
  return $id;
}

<<__EntryPoint>>
function main() :mixed{
  wat(42);
  wat(make_fbid2(5));
  wat("fail");
  echo "Done\n";
}
