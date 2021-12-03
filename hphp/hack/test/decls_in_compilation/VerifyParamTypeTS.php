<?hh

class NoGenerics {}
class ErasedGenericsClass<T> {}
class ReifiedGenericsClass<reify T> {}

function test(
  NoGenerics $one,
  ErasedGenericsClass<int> $t2,
  ReifiedGenericsClass<int> $t3,
): void {}
