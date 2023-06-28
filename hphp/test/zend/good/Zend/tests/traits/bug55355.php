<?hh

// A trait that has a abstract function
trait ATrait {
    function bar() :mixed{
        $this->foo();
    }
    abstract function foo():mixed;
}

// A class on the second level in the
// inheritance chain
class Level2Impl {
    function foo() :mixed{}
}

class Level1Indirect extends Level2Impl {}

// A class on the first level in the
// inheritance chain
class Level1Direct {
    function foo() :mixed{}
}

// Trait Uses

class Direct {
    use ATrait;
    function foo() :mixed{}
}

class BaseL2 extends Level1Indirect {
    use ATrait;
}

class BaseL1 extends Level1Direct {
    use ATrait;
}
<<__EntryPoint>> function main(): void {
echo 'DONE';
}
