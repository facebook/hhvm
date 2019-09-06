<?hh

trait TestTrait {
    public static function test() {
        return 'Test';
    }
}

<<__DynamicallyConstructible>>
class A {
    use TestTrait;
}
<<__EntryPoint>> function main(): void {
$class = "A";
echo $class::test();
}
