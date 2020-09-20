<?hh

<<__EntryPoint>>
function test(): void {
  $x = dict['a' => 'b'];
  $c = () ==> $x['a'];
  var_dump($c);
  $c();
  $rc = new ReflectionClass($c);
  $c2 = $rc->newInstanceWithoutConstructor();
  var_dump($c2);
  $c2();
}
