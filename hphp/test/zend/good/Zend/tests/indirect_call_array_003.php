<?hh

class foo {
    public function __call($a, $b) {
        printf("From %s:\n", __METHOD__);
        var_dump($a);
        var_dump($this);
    }
}
<<__EntryPoint>> function main(): void {
$foo = new foo;
$arr = varray[$foo, 'abc'];
$arr();
}
