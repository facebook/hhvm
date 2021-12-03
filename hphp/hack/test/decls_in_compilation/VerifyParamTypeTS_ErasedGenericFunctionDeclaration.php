<?hh

class NoGenerics {}
class ErasedGenericsClass<T> {}
class ReifiedGenericsClass<reify T> {}

function test<T>(
  NoGenerics $one,
  ErasedGenericsClass<T> $t2,
  ReifiedGenericsClass<int> $t3,
): void {}
