<?hh // strict

// All happy paths

class D {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function test(): void {
  $lambda1 = (...$y) ==> $y[0]->bar();
  $lambda1(new D("first"));

  $lambda2 = ($i, $str, $d, ...$d_var) ==> $d_var[0]->bar();
  $lambda2(1, "hello", new D("first"), new D("second"));
  $lambda2(1, "hello", new D("first"));

  $lambda3 = (int $i, string $str, D $d, D ...$d_var) ==> $d_var[0]->bar();
  $lambda3(1, "hello", new D("first"), new D("second"));
  $lambda3(1, "hello", new D("first"));

  $closure1 = function(...$y): int {
    return $y[0]->bar();
  };
  $closure1(new D("first"));

  $closure2 = function($i, $str, $d, ...$d_var): int {
    return $d_var[0]->bar();
  };
  $closure2(1, "hello", new D("first"), new D("second"));
  $closure2(1, "hello", new D("first"));

  $closure3 = function(int $i, string $str, D $d, D ...$d_var): int {
    return $d_var[0]->bar();
  };
  $closure3(1, "hello", new D("first"), new D("second"));
  $closure3(1, "hello", new D("first"));
}
