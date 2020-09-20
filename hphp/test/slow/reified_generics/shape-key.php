<?hh

class T {
  const string A = "from_T";
}

function f<reify T>(): void {
  $x = shape(
    T::A => 4
  );
  var_dump($x);
}

class D {
  const string A = "from_D";
}

class E {
  const string A = "from_E";
}
<<__EntryPoint>> function main(): void {
f<D>();
f<E>();
}
