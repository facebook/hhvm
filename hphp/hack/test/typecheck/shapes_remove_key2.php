<?hh

function test_open1(shape('a' => int, 'b' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}

function test_open2(shape('a' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}

function test_open3(shape('b' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}

function test_closed1(shape('a' => int, 'b' => int) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}

function test_closed2(shape('a' => int) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}

function test_closed3(shape('b' => int) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $s['a'];
  $s['b'];
}
