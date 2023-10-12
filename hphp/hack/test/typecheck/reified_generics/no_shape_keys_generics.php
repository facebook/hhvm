<?hh // strict

class C {
  const string A = "key_c";
}

function f<reify T as C>(): void {
  $x = shape(
    T::A => 4
  );
}
