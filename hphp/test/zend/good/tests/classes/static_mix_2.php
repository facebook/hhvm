<?hh

class pass {
    function show() :mixed{
        echo "Call to function pass::show()\n";
    }
}

class fail extends pass {
    static function show() :mixed{
        echo "Call to function fail::show()\n";
    }
}
<<__EntryPoint>> function main(): void {
$t = new pass();
$t->show();
fail::show();

echo "Done\n"; // shouldn't be displayed
}
