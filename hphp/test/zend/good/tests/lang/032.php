<?hh
class A {
    function foo() :mixed{}
}

class B extends A {
    function foo() :mixed{}
}

class C extends B {
    function foo() :mixed{}
}

class D extends A {
}

class F extends D {
    function foo() :mixed{}
}

// Following class definition should fail, but cannot test
/*
class X {
    function foo() {}
    function foo() {}
}
*/
<<__EntryPoint>> function main(): void {
echo "OK\n";
}
