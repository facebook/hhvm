<?hh
class Test {
    function __copy()
    {
        $string = PHP_VERSION;
        $version = $string[0];
        if($string < 5)
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
