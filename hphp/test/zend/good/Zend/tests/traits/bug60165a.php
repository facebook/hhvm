<?hh

trait A {
    public function bar() {}
}

class MyClass {
    use A {
        nonExistent as barA;
    }
}

<<__EntryPoint>> function main(): void {}
