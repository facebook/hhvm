<?hh

interface I {}
function hof((function (mixed): bool) $f): void {}
function hof_like(~(function (mixed): bool) $f): void {}
function hof_null(?(function (mixed): bool) $f): void {}
function hof_like_null(~?(function (mixed): bool) $f): void {}

function test(): void {
  hof(($i) ==> $i is I);
  hof_like(($i) ==> $i is I);
  hof_null(($i) ==> $i is I);
  hof_like_null(($i) ==> $i is I);
}
