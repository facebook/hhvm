<?hh

class Both<Ta, reify Tb> {}

function f<T>(): void {
  3 as Both<T, _>;

  4 as Both<_, T>;
}

function g<reify T>(): void {
  3 as Both<T, _>;

  4 as Both<_, T>;
}

function h<<<__Enforceable>> reify T>(): void {
  3 as Both<T, _>;

  4 as Both<_, T>;
}
