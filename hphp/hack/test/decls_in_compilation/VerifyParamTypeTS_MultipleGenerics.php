<?hh

class NoGenerics {}
class ErasedGenericsClass<T, Ta, Tb> {}
class ReifiedGenericsClass<T, Ta, reify Tb> {}

function test(
  NoGenerics $one,
  ErasedGenericsClass<int, int, int> $t2,
  ReifiedGenericsClass<int, int, int> $t3,
): void {}
