<?hh

<<__EntryPoint>>
function main_1353() {
  $r1 = new ReflectionClass('Test1353\\C');
  $r2 = new ReflectionMethod('Test1353\\M', 'foo');
  var_dump($r1->getName());
  var_dump($r2->getName());
}
