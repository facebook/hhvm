<?hh
abstract class Base {}
final class C extends Base {}
interface I {}

function expect_C(C $b) : void {}

function bounded_generic_fun<T as Base>(T $x): void {
  if ($x is C) {
    expect_C($x);
  }
  hh_show($x);
}

function simple(mixed $m) : void {
  if ($m is C || $m is I) {
  } else if ($m is Base) {}
  hh_show($m);
}

function generic<T>(T $m) : void {
  if ($m is C || $m is I) {
  }
  hh_show($m);
}
