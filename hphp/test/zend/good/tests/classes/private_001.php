<?hh

class pass {
    private static function show() :mixed{
        echo "Call show()\n";
    }

    public static function do_show() :mixed{
        pass::show();
    }
}
<<__EntryPoint>> function main(): void {
pass::do_show();
pass::show();

echo "Done\n"; // shouldn't be displayed
}
