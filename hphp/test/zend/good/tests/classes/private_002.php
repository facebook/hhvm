<?hh

class pass {
    private static function show() :mixed{
        echo "Call pass::show()\n";
    }

    public static function do_show() :mixed{
        pass::show();
    }
}

class fail {
    public static function show() :mixed{
        echo "Call fail::show()\n";
        pass::show();
    }
}

<<__EntryPoint>> function main(): void {
pass::do_show();

fail::show();

echo "Done\n"; // shouldn't be displayed
}
