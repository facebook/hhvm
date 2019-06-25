<?hh

class Test implements Countable
{
    function count()
    {
        return 4;
    }
};
<<__EntryPoint>> function main(): void {
$a = new Test;

var_dump(count($a));

echo "===DONE===\n";
}
