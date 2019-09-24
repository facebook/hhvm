<?hh

class A {
 function test(A $a) {
 $a->foo();
}
 function foo() {
 print 'foo';
}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
