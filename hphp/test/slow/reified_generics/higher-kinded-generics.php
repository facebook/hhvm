<?hh // strict

class C<reify T> {}

function f<reify T>() {
  new T<T>();
}

f<C>();
