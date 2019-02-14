<?hh

function f<reify T1, T2, reify T3>() {
}

function g<T>() {
  f<int, string, T>();
}

g();
