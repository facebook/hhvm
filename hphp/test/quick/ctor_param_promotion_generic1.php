<?hh

class A {
  function __construct<T>(public Vector<Map<string, T>> $f) {}
}

<<__EntryPoint>> function main(): void {}
