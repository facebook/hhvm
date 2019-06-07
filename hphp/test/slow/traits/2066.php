<?hh

trait T {
  function fruit() {
    yield 'apple';
    yield 'banana';
  }
}
class C1 {
 use T;
 }
class C2 {
 use T;
 }

<<__EntryPoint>>
function main_2066() {
$o = new C1;
foreach ($o->fruit() as $fruit) {
  var_dump($fruit);
}
}
