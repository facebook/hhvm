<?hh

function f<T1, reify T2, <<__Enforceable>> reify T3>(): void {
  1 as T1; // error
  2 as ~T1; // error
  3 as T2; // error
  4 as ~T2; // error
  5 as T3; // OK
  6 as ~T3; // OK
}
