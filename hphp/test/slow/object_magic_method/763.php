<?hh
function test($t) :mixed{
  var_dump('test:'.$t);
  (new $t(1,2))->foo();
}
<<__EntryPoint>>
function entrypoint_763(): void {

  if (isset($g)) {
    include '763-1.inc';
  } else {
    include '763-2.inc';
  }

  include '763-classes.inc';

  test('X');
  test('Y');
  test('Z');
}
