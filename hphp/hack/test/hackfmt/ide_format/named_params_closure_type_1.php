<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function test1((function(int,
named                       int       $x    , optional       named   int  $y ): void) $f): void {}
function test2((function(
  int, named it          $x, optional named int $y, optional
  named int $j2afsjsdjfklasjflasdjfklajsflkjadsfkjasdfljasdlkfjdsklfjsdklsdajfsadklfd): void) $f): void {}

function main(): void {
  test1((int $_, named int $x, named int $y = 1) ==> {});
  test1((
    int $_, named int $x, named int $y = 1,
named int $j2afsjsdjfklasjflasdjfklajsflkjadsfkjasdfljasdlkfjdsklfjsdklsdajfsadklfd = 3
  ) ==> {});
}
