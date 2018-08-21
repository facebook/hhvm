<?hh // strict

class A {
  const type C = (function(): int);
}

class B extends A {
  const type C = (coroutine function(): int);
}
