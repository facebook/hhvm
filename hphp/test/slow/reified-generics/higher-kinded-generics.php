<?hh

class C<reify T> {}

function f<reify T>() :mixed{
  new T<T>();
}
<<__EntryPoint>> function main(): void {
f<C>();
}
