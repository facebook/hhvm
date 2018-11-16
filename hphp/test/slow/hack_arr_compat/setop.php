<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a) {
  echo "test----------------------------\n";

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

function test_append($a) {
  echo "test_append---------------------\n";

  $a[]++;
  ++$a[];
  $a[]--;
  --$a[];

  $a[10]++;
  ++$a[11];
  $a[12]--;
  --$a[13];

  $a[]++;
  ++$a[];
  $a[]--;
  --$a[];

  var_dump($a);
}

<<__EntryPoint>>
function main_setop() {
  test([]);
  test_append([]);

  test([1 => 'foo', 'baz' => 123]);
  test_append([1 => 'foo', 'baz' => 123]);
}
