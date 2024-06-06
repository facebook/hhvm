<?hh

interface I {}
trait T implements I {}

trait T1 {}
trait T2 { use T1; }

class A { use T; }
class B { use T1; }
class C { use T2; }

<<__EntryPoint>>
function main(): void {
  var_dump(is_a("T", "T", true));
  var_dump(is_a("T", "I", true));

  var_dump(is_a("T2", "T1", true));
  var_dump(is_a("T", "I", true));

  var_dump(is_a(new A(), "T", true));
  echo "Should be only true in file: ";
  var_dump(is_a(new A(), "I", true));

  var_dump(is_a(new B(), "T1", true));

  var_dump(is_a(new C(), "T2", true));
  var_dump(is_a(new C(), "T1", true));

}
