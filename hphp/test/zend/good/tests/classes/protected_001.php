<?hh

class pass {
    protected static function fail() :mixed{
        echo "Call fail()\n";
    }

    public static function good() :mixed{
        pass::fail();
    }
}
<<__EntryPoint>> function main(): void {
pass::good();
pass::fail();// must fail because we are calling from outside of class pass

echo "Done\n"; // shouldn't be displayed
}
