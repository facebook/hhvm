<?hh

class pass {
    private static function show() :mixed{
        echo "Call show()\n";
    }

    protected static function good() :mixed{
        pass::show();
    }
}

class fail extends pass {
    static function ok() :mixed{
        pass::good();
    }

    static function not_ok() :mixed{
        pass::show();
    }
}

<<__EntryPoint>> function main(): void {
ini_set("error_reporting", 2039);

fail::ok();
fail::not_ok(); // calling a private function

echo "Done\n"; // shouldn't be displayed
}
