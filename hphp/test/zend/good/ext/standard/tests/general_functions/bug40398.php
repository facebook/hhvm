<?hh

class Base
{
    function __construct($msg)
    {
        echo __METHOD__ . "($msg)\n";
    }
}

class Derived_1 extends Base
{
    public function __construct(...$args)
    {
        $x = vec[$this, 'Base::__construct']; $x(...$args);
    }
}

class Derived_2 extends Base
{
    public function __construct(...$args)
    {
        $x = vec[$this, 'parent::__construct']; $x(...$args);
    }
}

class Derived_3 extends Base
{
    public function __construct(...$args)
    {
        $x = 'Base::__construct'; $x(...$args);
    }
}

class Derived_4 extends Base
{
    public function __construct(...$args)
    {
        $x = 'parent::__construct'; $x(...$args);
    }
}

class Derived_5 extends Base
{
    public function __construct(...$args)
    {
        $x = vec['Base', '__construct']; $x(...$args);
    }
}

class Derived_6 extends Base
{
    public function __construct(...$args)
    {
        $x = vec['parent', '__construct']; $x(...$args);
    }
}
<<__EntryPoint>> function main(): void {
new Derived_1('1');
new Derived_2('2');
new Derived_3('3');
new Derived_4('4');
new Derived_5('5');
new Derived_6('6');

echo "===DONE===\n";
}
