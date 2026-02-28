<?hh

trait T {
  static function foo() :mixed{
    echo "I'm in class " . get_class(new self()) . "\n";
  }
}
class C {
 use T;
 }
class D extends C {
}
<<__EntryPoint>> function main(): void {
C::foo();
D::foo();
}
