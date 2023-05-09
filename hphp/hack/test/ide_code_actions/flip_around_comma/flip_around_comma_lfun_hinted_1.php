<?hh

function foo(): void {
  ((int $a, /*range-start*//*range-end*/int $b) ==> {

  })(1, 2);
}
