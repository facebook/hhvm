<?hh
class Test {
    function __copy()
:mixed    {
        $string = PHP_VERSION;
        $version = $string[0];
        if(HH\Lib\Legacy_FIXME\lt($string, 5))
        {
            return $this;
        }
        else
        {
            return clone $this;
        }
    }
}
<<__EntryPoint>> function main(): void {
$test = new Test();
$test2 = $test->__copy();
var_dump($test2);
}
