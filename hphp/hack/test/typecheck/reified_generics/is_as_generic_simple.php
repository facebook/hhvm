<?hh // strict

function f<T>(): void {
  3 as T;
}

function g<reify T>(): void {
  3 as T;
}

function h<<<__Enforceable>> reify T>(): void {
  3 as T;
}
