<?hh

type TS = shape('a' => int, string...);

function test_as(mixed $m):void {
  $m as shape('a' => int, string...);
}

function test_is(mixed $m):void {
  if ($m is shape('a' => int, string...)) {
  }
}

function test_as_2(mixed $m):void {
  $m as TS;
}

function test_is_2(mixed $m):void {
  if ($m is TS) {
  }
}
