<?hh

class A {
 function test(A $a) :mixed{
 $a->foo();
}
 function foo() :mixed{
 print 'foo';
}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
