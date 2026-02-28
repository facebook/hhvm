<?hh

trait TestTrait {
    public static function test() :mixed{
        return 'Forwarded '.A::test();
    }
}

class A {
    public static function test() :mixed{
        return "Test A";
    }
}

class B extends A {
    use TestTrait;
}
<<__EntryPoint>> function main(): void {
echo B::test();
}
