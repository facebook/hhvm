<?hh

class MyDisposable implements IDisposable {
  public function __dispose(): void {}
}

function my_inout1(inout int $x) { $x++; }
function my_inout2(inout int $x): int { $x++; return 0; }

function foo(): void {
  $x = 42;
  $x += 42;
  $x -= 42;
  $x *= 42;
  $x /= 42;
  $x %= 42;
  $x **= 42;
  $x .= 42;
  $x &= 42;
  $x |= 42;
  $x ^= 42;
  $x <<= 42;
  $x >>= 42;
  $x ??= 42;
  $x++;
  ++$x;
  $x--;
  --$x;
  $dict = dict[];
  for ($x = 42, $x = 43; false; $x = 44, $x = 45) {}
  using $x = new MyDisposable();
  using ($x = new MyDisposable(), $y = new MyDisposable()) {}
  my_inout1(inout $x);
  @my_inout1(inout $x);
  $y = @my_inout1(inout $x);
  my_inout1<int>(inout $x);
  $y = my_inout2(inout $x);
  $y = my_inout2<int>(inout $x);
  list($x1, $x2, list($x3, $x4)) = tuple(1, 2, tuple(3, 4));
}
