<?hh

class A {

    protected static $property = TRUE;

    protected static function method() :mixed{
        return TRUE;
    }

}

class B extends A {

    public function __construct() {

        var_dump(self::method());
        var_dump(parent::method());

        var_dump(self::$property);
        var_dump(parent::$property);

    }

}
<<__EntryPoint>> function main(): void {
new B;
}
