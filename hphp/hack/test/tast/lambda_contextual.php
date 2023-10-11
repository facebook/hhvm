<?hh

function takes_fun((function (int): num) $f): void {}

function test(): void {
  takes_fun($x ==> $x);
}
