<?hh

trait testTrait {
    public function testMethod() :mixed{
        if (1) {
            $letters1 = range('a', 'z', 1);
            $letters2 = range('A', 'Z', 1);
            $letters1 = 'foo';
            $letters2 = 'baarr';
            var_dump($letters1);
            var_dump($letters2);
        }
    }
}

class foo {
    use testTrait;
}
<<__EntryPoint>> function main(): void {
$x = new foo;
$x->testMethod();
}
