<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a) {
  $a[10]++;
  ++$a[11];
  $a[12]--;
  --$a[13];

  $a['a']++;
  ++$a['b'];
  $a['c']--;
  --$a['d'];

  $a[20] .= 'abc';
  $a[21] += 10;
  $a[22] |= true;

  $a['e'] .= 'abc';
  $a['f'] += 10;
  $a['g'] |= true;

  var_dump($a);
}


<<__EntryPoint>>
function main_setop() {
test([1 => 'foo', 'baz' => 123]);
}
