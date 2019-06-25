<?hh

trait TestTrait {
    public static function test() {
        return 'Forwarded '.A::test();
    }
}

class A {
    public static function test() {
        return "Test A";
    }
}

class B extends A {
    use TestTrait;
}
<<__EntryPoint>> function main(): void {
echo B::test();
}
