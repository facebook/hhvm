<?hh
class test
{
    public $a, $b;

    function __construct()
    {
        $this->a = 7;
        $this->b = 2;
    }

    function __sleep()
    {
        $this->b = 0;
    }
}
<<__EntryPoint>> function main(): void {
$t = darray[];
$t['one'] = 'ABC';
$t['two'] = new test();

var_dump($t);

$s =  @serialize($t);
echo $s . "\n";

var_dump(unserialize($s));
}
