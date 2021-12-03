<?hh

namespace MyTestNamespace {

class NoGenerics {}
class ErasedGenericsClass<T> {}
class ReifiedGenericsClass<reify T> {}

function test(
  NoGenerics $one,
  ErasedGenericsClass<int> $t2,
  ReifiedGenericsClass<int> $t3,
  \ReifiedGenericsClass<int> $t4,
): void {}

}
