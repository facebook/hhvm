<?hh

class NoGenerics {}
class ErasedGenericsClass<T> {}
class ReifiedGenericsClass<reify T> {}

function test<reify T>(
  NoGenerics $one,
  ErasedGenericsClass<T> $t2,
  ReifiedGenericsClass<T> $t3,
): void {}
