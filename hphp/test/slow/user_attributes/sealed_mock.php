<?hh

<<__Sealed(WhoEvenCares::class)>>
class MyClass {}

<<__Sealed(WhoEvenCares::class)>>
interface MyInterface {}

<<__Sealed(WhoEvenCares::class)>>
trait MyTrait {}

<<__MockClass>>
final class MyMockClass1 extends MyClass {}

<<__MockClass>>
final class MyMockClass2 implements MyInterface {}

<<__MockClass>>
final class MyMockClass3 { use MyTrait; }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
