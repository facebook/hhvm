<?hh

class NoGenerics {}
class WarnReifiedGenericsClass<T, Ta, <<__Warn>> reify Tb> {}
class SoftReifiedGenericsClass<T, Ta, <<__Soft>> reify Tb> {}

function test(
  NoGenerics $one,
  WarnReifiedGenericsClass<int, int, int> $t2,
  SoftReifiedGenericsClass<int, int, int> $t3,
): void {}
