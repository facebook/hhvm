<?hh

function f(int &$x, string $y, ...$zs): void {}

class A<T as num> {
  public function g(T &$x): num {
    $x += 1;
    return $x;
  }
  public static function h(string $t): void {}
}

function test(): void {
  $i = 42;
  $s = 'baz';

  $f = fun('f');
  $f($i, $s);
  $f(&$i, $s);
  $f($i, &$s);
  $f(&$i, &$s);
  $f(&$i, $s, $i, &$i, $s, $i, &$s);

  $a = new A();
  $g = inst_meth($a, 'g');
  $g($i);
  $g(&$i);

  $h = class_meth(A::class, 'h');
  $h($s);
  $h(&$s);

  $l = (int $x, string &$y) ==> $y .= "foo$x";
  $l($i, $s);
  $l($i, &$s);
  $l(&$i, $s);
  $l(&$i, &$s);

  $m = function(arraykey $z): string {
    return (string)$z;
  };
  $m($i);
  $m(&$i);
}
