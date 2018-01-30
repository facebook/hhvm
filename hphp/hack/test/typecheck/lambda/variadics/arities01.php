<?hh // strict

class D {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function expect_D((function(D...): int) $f): void {
  $f(new D("first"));
}

function expect_D2((function(D): int) $f): void {
  $f(new D("first"));
}

function test(): void {
  expect_D((int $a, D ...$y) ==> $y[0]->bar()); //Too many mandatory args

  $lambda = (int $a, D ...$y) ==> $y[0]->bar(); //Too many mandatory args
  expect_D($lambda);

  expect_D2((D $d1, D $d2, D ...$y) ==> $y[0]->bar()); //Too many mandatory args

  expect_D(() ==> 1); //Expects variaidc parameters

  $lambda = () ==> 1;
  expect_D($lambda); //Expects variaidc parameters

  expect_D2(() ==> 1); //Too few args

  $lambda = () ==> 1;
  expect_D2($lambda); //Too few args
}
