<?hh // strict

// Because of the ordering here, things will fail to load if we don't
// properly mark A and C as not being hoistable
class A { use B; }
trait B {}

interface I1 {}
interface I2 {}

class C extends D implements I1, I2 {}
class D {}

function test(): void {
  var_dump(new A());
  var_dump(new C());
  var_dump(new D());
}
