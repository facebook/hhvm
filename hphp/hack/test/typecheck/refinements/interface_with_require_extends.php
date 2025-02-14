<?hh

abstract class MyFoo {}

interface IMyFoo {
  require extends MyFoo;
}
interface IMyBar {
  require extends MyFoo;
}
interface IMyBaz {
  require extends MyFoo;
}

function expect_imybaz(IMyBaz $x): void {}

function foo(HH\Lib\Ref<IMyFoo> $x_ref): void {
  $x = $x_ref->value;
  invariant($x is IMyBaz || $x is IMyBar, '');

  if ($x is IMyBaz) {
    // just to trigger a split and re-union
  } else {
    // ditto
  }

  if (!$x is IMyBar) {
    expect_imybaz($x);
  }
}
