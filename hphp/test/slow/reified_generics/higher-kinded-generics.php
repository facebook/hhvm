<?hh // strict

class C<reify T> {}

function f<reify T>() {
  new T<reify T>();
}

f<reify C>();
