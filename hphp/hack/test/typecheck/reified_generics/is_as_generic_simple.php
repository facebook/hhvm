<?hh // strict

function f<T>(): void {
  3 as T;
  4 as (int, T);
}

function g<reify T>(): void {
  3 as T;
  4 as (int, T);
}

function h<<<__Enforceable>> reify T>(): void {
  3 as T;
  4 as (int, T);
}
