<?hh

/*
abstract class AC {}

new AC;     // Cannot instantiate abstract class AC
*/

interface i1 {}
interface i2 {}
class C1 {}
class C2 extends C1 implements i1, i2 {}
<<__EntryPoint>>
function main_entry(): void {

  error_reporting(-1);

  $c = new C2;
  var_dump($c);
}
