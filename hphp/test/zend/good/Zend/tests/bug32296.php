<?hh
abstract class space{
    function __construct(){}
    abstract protected function unfold():mixed;
}

abstract class shape_ extends space{
    private function x1() :mixed{}
    protected final function unfold():mixed{}
}

abstract class quad extends shape_ {
    private function x2() :mixed{}
    function buggy():mixed{
        $c = get_class($this);
        $a = get_class_methods(get_class($this));
        $b = get_class_methods($this);
        print($c."\n".'a:');
        print_r($a);
        print('b:');
        print_r($b);
    }
}

class square extends quad{}
<<__EntryPoint>> function main(): void {
$a = new square();
$a->buggy();
print_r(get_class_methods("square"));
print_r(get_class_methods($a));
}
