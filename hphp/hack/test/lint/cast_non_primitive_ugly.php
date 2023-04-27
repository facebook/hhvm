<?hh

// This file is for the test cases of the non-primitive cast linter where there
// should be a lint but doing so would lead to too much noise. It would be
// desirable to eventually start producing lints for the test cases in this
// file.

function cast_non_primitive_ugly1<<<__Explicit>> T>(T $t): void {
  (int) $t;
}

function cast_non_primitive_ugly2(dynamic $dyn): void {
  (int) $dyn;
}

function cast_non_primitive_ugly3(mixed $m): void {
  (int) $m;
}

abstract class AC {
  abstract const type TRet;
  abstract public function make(): this::TRet;
}

function cast(AC $obj): void {
  (int) $obj->make();
}
