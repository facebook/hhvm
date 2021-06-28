<?hh
class X
{
    // Static and instance array using class constants
    public static $sa_x = darray[B::KEY => B::VALUE];
    public $a_x = darray[B::KEY => B::VALUE];
}

class B
{
    const KEY = "key";
    const VALUE = "value";

    // Static and instance array using class constants with self
    public static $sa_b = darray[self::KEY => self::VALUE];
    public $a_b = darray[self::KEY => self::VALUE];
}

class C extends B
{
    // Static and instance array using class constants with parent
    public static $sa_c_parent = darray[parent::KEY => parent::VALUE];
    public $a_c_parent = darray[parent::KEY => parent::VALUE];

    // Static and instance array using class constants with self (constants should be inherited)
    public static $sa_c_self = darray[self::KEY => self::VALUE];
    public $a_c_self = darray[self::KEY => self::VALUE];

    // Should also include inherited properties from B.
}
<<__EntryPoint>> function main(): void {
echo "\nStatic properties:\n";
var_dump(X::$sa_x, B::$sa_b, C::$sa_b, C::$sa_c_parent, C::$sa_c_self);

echo "\nInstance properties:\n";
$x = new X;
$b = new B;
$c = new C;
var_dump($x, $b, $c);
}
