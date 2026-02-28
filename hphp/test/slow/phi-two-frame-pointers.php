<?hh

<<__NEVER_INLINE>>
function mixed_value($x) :mixed{
  if ($x === 'null') return null;
  if ($x === 'varray') return vec['value'];
  if ($x === 'darray') return dict[0 => 'value'];
}

function nesting_level_1($x) :mixed{
  return mixed_value($x)[0];
}

function nesting_level_0($x) :mixed{
  return nesting_level_1($x);
}

<<__NEVER_INLINE>>
function test($x) :mixed{
  var_dump(nesting_level_0($x));
}

<<__EntryPoint>>
function main() :mixed{
  test('null');
  test('null');
  test('varray');
}
