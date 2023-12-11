<?hh

class MyArrayIterator extends ArrayIterator
{
    static protected $fail = 0;
    public $state;

    static function fail($state, $method)
:mixed    {
        if (self::$fail == $state)
        {
            throw new Exception("State $state: $method()");
        }
    }

    function __construct()
    {
        $this->state = MyArrayIterator::$fail;
        self::fail(0, __FUNCTION__);
        parent::__construct(vec[1, 2]);
        self::fail(1, __FUNCTION__);
    }

    function rewind()
:mixed    {
        self::fail(2, __FUNCTION__);
        return parent::rewind();
    }

    function valid()
:mixed    {
        self::fail(3, __FUNCTION__);
        return parent::valid();
    }

    function current()
:mixed    {
        self::fail(4, __FUNCTION__);
        return parent::current();
    }

    function key()
:mixed    {
        self::fail(5, __FUNCTION__);
        return parent::key();
    }

    function next()
:mixed    {
        self::fail(6, __FUNCTION__);
        return parent::next();
    }

    static function test($func, $skip = null)
:mixed    {
        echo "===$func===\n";
        self::$fail = 0;
        while(self::$fail < 10)
        {
            try
            {
                var_dump($func(new MyArrayIterator()));
                break;
            }
            catch (Exception $e)
            {
                echo $e->getMessage() . "\n";
            }
            if (isset($skip[self::$fail]))
            {
                self::$fail = $skip[self::$fail];
            }
            else
            {
                self::$fail++;
            }
        }
    }
}
<<__EntryPoint>> function main(): void {
MyArrayIterator::test('iterator_to_array');
MyArrayIterator::test('iterator_count', dict[3 => 6]);

echo "===DONE===\n";
}
