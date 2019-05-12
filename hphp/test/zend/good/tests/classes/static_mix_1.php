<?php

class pass {
    static function show() {
        echo "Call to function pass::show()\n";
    }
}

class fail extends pass {
    function show() {
        echo "Call to function fail::show()\n";
    }
}
<<__EntryPoint>> function main() {
pass::show();
fail::show();

echo "Done\n"; // shouldn't be displayed
}
