<?hh

class first {
    private static function show() :mixed{
        echo "Call show()\n";
    }

    public static function do_show() :mixed{
        first::show();
    }
}

class second extends first {
}

class third extends second {
}

class fail extends third {
    // cannot be redeclared
    static function show() :mixed{
        echo "Call show()\n";
    }
}

<<__EntryPoint>> function main(): void {
first::do_show();

second::do_show();

third::do_show();

echo "Done\n";
}
