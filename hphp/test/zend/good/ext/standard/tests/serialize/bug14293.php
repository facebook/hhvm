<?hh
class t
{
    function __construct()
    {
        $this->a = 'hello';
    }

    function __sleep()
:mixed    {
        echo "__sleep called\n";
        return vec['a','b'];
    }
}
<<__EntryPoint>> function main(): void {
$t = new t();
$data = serialize($t);
echo "$data\n";
$t = unserialize($data);
var_dump($t);
}
