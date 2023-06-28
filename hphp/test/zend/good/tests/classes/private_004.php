<?hh

class pass {
    private static function show() :mixed{
        echo "Call show()\n";
    }

    public static function do_show() :mixed{
        pass::show();
    }
}

class fail extends pass {
    static function do_show() :mixed{
        fail::show();
    }
}
<<__EntryPoint>> function main(): void {
pass::do_show();
fail::do_show();

echo "Done\n"; // shouldn't be displayed
}
