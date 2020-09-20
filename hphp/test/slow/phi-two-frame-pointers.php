<?hh

<<__NEVER_INLINE>>
function mixed_value($x) {
  if ($x === 'null') return null;
  if ($x === 'varray') return varray['value'];
  if ($x === 'darray') return darray[0 => 'value'];
}

function nesting_level_1($x) {
  return mixed_value($x)[0];
}

function nesting_level_0($x) {
  return nesting_level_1($x);
}

<<__NEVER_INLINE>>
function test($x) {
  var_dump(nesting_level_0($x));
}

<<__EntryPoint>>
function main() {
  test('null');
  test('null');
  test('varray');
}
