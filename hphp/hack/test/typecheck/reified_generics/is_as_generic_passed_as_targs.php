<?hh // strict

class Both<Ta, reify Tb> {}

function f<T>(): void {
  3 as Both<T, _>;

  4 as Both<_, T>;
}

function g<reify T>(): void {
  3 as Both<T, _>;

  // This one is important. Consider the case of
  //
  //   g<Both<int, string>>()
  //
  // which is a valid type argument, but becomes
  //
  //   4 as Both<int, string>;
  //
  // which is obviously illegal, because int is erased.
  // This is why T must be enforceable.
  4 as Both<_, T>;
}

function h<<<__Enforceable>> reify T>(): void {
  3 as Both<T, _>;

  4 as Both<_, T>;
}
