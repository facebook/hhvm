<?hh // strict

class C<reify T> {}

function f<reify T>() {
  new T<T>();
}
<<__EntryPoint>> function main(): void {
f<C>();
}
