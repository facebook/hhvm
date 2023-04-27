<?hh

module b;

internal enum Foo: string as string {
  BAR = 'bar';
}

<<__SoftInternal>>
internal enum SoftFoo: string as string {
  BAR = 'bar';
}

internal class C {}

internal enum class EC: C {
  C BAR = new C();
}

<<__SoftInternal>>
internal enum class SoftEC: C {
  C BAR = new C();
}
