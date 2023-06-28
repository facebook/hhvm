<?hh

class Test implements Countable
{
    function count()
:mixed    {
        return 4;
    }
}
<<__EntryPoint>> function main(): void {
$a = new Test;

var_dump(count($a));

echo "===DONE===\n";
}
