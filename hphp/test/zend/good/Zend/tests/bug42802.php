<?hh
namespace foo;

class bar {
}

function test1(bar $bar) :mixed{
    echo "ok\n";
}

function test2(\foo\bar $bar) :mixed{
        echo "ok\n";
}
function test3(\foo\bar $bar) :mixed{
        echo "ok\n";
}
function test4(\Exception $e) :mixed{
    echo "ok\n";
}
function test5(\bar $bar) :mixed{
        echo "bug\n";
}
<<__EntryPoint>> function main(): void {
$x = new bar();
$y = new \Exception();
test1($x);
test2($x);
test3($x);
test4($y);
test5($x);
}
