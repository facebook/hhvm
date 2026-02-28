<?hh

interface I {}
abstract class C implements I {}
final class D extends C {}
final class E extends C {}
final class F implements I {}

type TI = I;
type TC = C;
type TD = D;

function is_I(mixed $x): void {
  if ($x is TI) {
    echo "I\n";
  } else {
    echo "not I\n";
  }
}

function is_C(mixed $x): void {
  if ($x is TC) {
    echo "C\n";
  } else {
    echo "not C\n";
  }
}

function is_D(mixed $x): void {
  if ($x is TD) {
    echo "D\n";
  } else {
    echo "not D\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_class() :mixed{
$d = new D();
is_D($d);
is_C($d);
is_I($d);
echo "\n";
$s = new stdClass();
is_D($s);
is_C($s);
is_I($s);
echo "\n";
$e = new E();
is_D($e);
is_C($e);
is_I($e);
echo "\n";
$f = new F();
is_D($f);
is_C($f);
is_I($f);
}
