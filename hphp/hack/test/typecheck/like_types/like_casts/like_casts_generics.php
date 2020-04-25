<?hh

function f<T1, reify T2>(): void {
  1 as T1; // error
  2 as ~T1; // OK
  3 as T2; // error
  4 as ~T2; // OK
}
