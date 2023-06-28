<?hh
class foo
{
    function __toString()[]
:mixed    {
        return "Object";
    }
}

<<__EntryPoint>> function main(): void {
$a = new foo();

$arr = darray[0=>$a, 1=>$a];
var_dump(implode(",",$arr));
var_dump($arr);
}
