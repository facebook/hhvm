<?hh
//new D1;   // Fatal error: Class 'D' not found

class D1 extends C1 {}
class C1 {}

//class C2 implements I2 {} // Fatal error: Interface 'I2' not found
interface I2 extends I1 {}  // OK; finds a forward declaration
interface I1 {}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  new C1;     // OK; finds a forward declaration
}
