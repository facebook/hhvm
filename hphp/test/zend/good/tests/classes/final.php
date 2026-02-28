<?hh

class first {
    function show() :mixed{
        echo "Call to function first::show()\n";
    }
}

class second extends first {
    final function show() :mixed{
        echo "Call to function second::show()\n";
    }
}

<<__EntryPoint>> function main(): void {
$t = new first();
$t->show();

$t2 = new second();
$t2->show();

echo "Done\n";
}
