<?hh

class C {
  const string HELLO = "hi";
}

function f<T as C>(classname<T> $x): void {
  hh_show($x::HELLO);
}

function g<reify T as C>(): void {
  hh_show(T::HELLO);
}
