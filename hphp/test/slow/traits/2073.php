<?hh

trait T {
  function info() :AsyncGenerator<mixed,mixed,void>{
    yield __METHOD__;
    yield __CLASS__;
    yield __TRAIT__;
  }
}
class C1 {
 use T;
 }
class C2 {
 use T;
 }

<<__EntryPoint>>
function main_2073() :mixed{
$o1 = new C1;
foreach ($o1->info() as $info) {
  var_dump($info);
}
$o2 = new C2;
foreach ($o2->info() as $info) {
  var_dump($info);
}
}
