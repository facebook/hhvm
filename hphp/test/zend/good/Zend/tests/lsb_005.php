<?hh

class TestA {
    public static function test() :mixed{
        echo get_class(new static()) . "\n";
        TestB::test();
        echo get_class(new static()) . "\n";
        TestC::test();
        echo get_class(new static()) . "\n";
        TestBB::test();
        echo get_class(new static()) . "\n";
    }
}

class TestB {
    public static function test() :mixed{
        echo get_class(new static()) . "\n";
        TestC::test();
        echo get_class(new static()) . "\n";
    }
}

class TestC {
    public static function test() :mixed{
        echo get_class(new static()) . "\n";
    }
}

class TestBB extends TestB {
}
<<__EntryPoint>> function main(): void {
TestA::test();
echo "==DONE==";
}
