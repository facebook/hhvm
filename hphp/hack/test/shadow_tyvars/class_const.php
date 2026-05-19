<?hh

function takes_int(int $_): void {}

function test_class_const(dynamic $d): void {
  $x = $d::MY_CONST;
  takes_int($x);
}
