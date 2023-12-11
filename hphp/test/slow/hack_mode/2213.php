<?hh
function vidx<X>(blarg<X> $list, int $idx):X {
  return $list->d[$idx];
}
function pair<X,Y>(X $x, Y $y):(X,Y) {
 return vec[$x, $y];
 }
function car<X,Y>((X,?Y) $pair):X {
  return $pair[0];
}
interface Face<A> {
}
class blarg<X> {
 function __construct($x) {
 $this->d = $x;
 }
 }
function blarg<X>(...$args):blarg<X> {
 return new blarg($args);
 }

class Foo<X> implements Face<X> {
  const string BLEH = "b";
}

function right_shift_hack(Foo<Foo<Foo<Foo<Foo<Foo<Foo<Foo<Foo<Foo<Foo>,Foo>>,Foo>>>,Foo>>>> $bonk,
         (function(Foo,Bar):C) $d) :mixed{
}


<<__EntryPoint>>
function main_2213() :mixed{
$blork = pair('c', '-');

$a = blarg('a','aa','aaa');
$d = (function():string{
return 'd';
}
);
echo vidx($a, 0), Foo::BLEH, car($blork), $d();
}
