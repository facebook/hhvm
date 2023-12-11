<?hh

function get() :mixed{
  return dict[
    'foo' => 'bar',
    'baz' => 24,
    13 => 42,
    'heh' => 1.0
  ];
}

function a() :mixed{ return get()['foo']; }
function b() :mixed{ return get()['baz']; }
function c() :mixed{ return get()[13]; }
function d() :mixed{ return get()['13']; }
function e() :mixed{ return get()['13foo']; }

function main() :mixed{
  var_dump(a() === 'bar');
  var_dump(b() === 24);
  var_dump(c() === 42);
  try { var_dump(d() === null); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(e() === null); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main_array_008() :mixed{
main();
}
