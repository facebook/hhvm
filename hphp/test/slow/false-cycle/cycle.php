<?hh

class A1 extends B2 { use T3; }
class A2 implements I { use T1; }
trait T2 { require extends B2; }

<<__EntryPoint>>
function main() {
  var_dump(new A1);
  var_dump(new B1);
}
