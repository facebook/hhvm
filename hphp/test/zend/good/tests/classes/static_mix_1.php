<?hh

class pass {
    static function show() :mixed{
        echo "Call to function pass::show()\n";
    }
}

class fail extends pass {
    function show() :mixed{
        echo "Call to function fail::show()\n";
    }
}
<<__EntryPoint>> function main(): void {
pass::show();
fail::show();

echo "Done\n"; // shouldn't be displayed
}
