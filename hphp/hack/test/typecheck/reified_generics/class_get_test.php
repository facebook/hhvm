<?hh

class C {
  public static string $hello = "hi";
}

function f<T as C>(classname<T> $x): void {
  hh_show($x::$hello);
}

function g<reify T as C>(): void {
  hh_show(T::$hello);
}
