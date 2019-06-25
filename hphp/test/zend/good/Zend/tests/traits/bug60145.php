<?hh

trait foo {

}

interface MyInterface {
    use foo;

    public function b();

}

<<__EntryPoint>> function main(): void {}
