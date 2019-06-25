<?hh

class bar {
}

class foo {
    public function test() {
        class_alias('bar', 'static');
        return new static;
    }
}
<<__EntryPoint>> function main(): void {
$a = new foo;
var_dump($a->test());
}
