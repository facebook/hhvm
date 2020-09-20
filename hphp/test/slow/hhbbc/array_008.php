<?hh

function get() {
  return darray[
    'foo' => 'bar',
    'baz' => 24,
    13 => 42,
    'heh' => 1.0
  ];
}

function a() { return get()['foo']; }
function b() { return get()['baz']; }
function c() { return get()[13]; }
function d() { return get()['13']; }
function e() { return get()['13foo']; }

function main() {
  var_dump(a() === 'bar');
  var_dump(b() === 24);
  var_dump(c() === 42);
  try { var_dump(d() === null); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(e() === null); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main_array_008() {
main();
}
