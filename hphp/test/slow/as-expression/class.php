<?hh

interface I {}
abstract class C implements I {}
final class D extends C {}
final class E extends C {}
final class F implements I {}

function as_I(mixed $x): void {
  try {
    var_dump($x as I);
  } catch (TypeAssertionException $_) {
    echo "not I: ".get_class($x)."\n";
  }
}

function as_C(mixed $x): void {
  try {
    var_dump($x as C);
  } catch (TypeAssertionException $_) {
    echo "not C: ".get_class($x)."\n";
  }
}

function as_D(mixed $x): void {
  try {
    var_dump($x as D);
  } catch (TypeAssertionException $_) {
    echo "not D: ".get_class($x)."\n";
  }
}


<<__EntryPoint>>
function main_class() :mixed{
$d = new D();
as_D($d);
as_C($d);
as_I($d);
echo "\n";
$s = new stdClass();
as_D($s);
as_C($s);
as_I($s);
echo "\n";
$e = new E();
as_D($e);
as_C($e);
as_I($e);
echo "\n";
$f = new F();
as_D($f);
as_C($f);
as_I($f);
}
