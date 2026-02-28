<?hh

abstract class A {}
class C extends A {}
class D extends A {}

enum BadEnum: class<A> {
  C = C::class;
  D = D::class;
}

enum class GoodEnumClass: class<A> {
  class<A> C = C::class;
  class<A> D = D::class;
}
