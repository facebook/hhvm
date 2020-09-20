<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Pass array_key_exists() an object with private and protected properties
 */

class myClass {
    public $var1;
    protected $var2;
    private $var3;

    function __construct($a, $b, $c = null) {
        $this->var1 = $a;
        $this->var2 = $b;
        if (!is_null($c)) {
            $this->var3 = $c;
        }
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : object functionality ***\n";

echo "\n-- Do not assign a value to \$class1->var3 --\n";
$class1 = new myClass ('a', 'b');
echo "\$key = var1:\n";
var_dump(array_key_exists('var1', $class1));
echo "\$key = var2:\n";
var_dump(array_key_exists('var2', $class1));
echo "\$key = var3:\n";
var_dump(array_key_exists('var3', $class1));
echo "\$class1:\n";
var_dump($class1);

echo "\n-- Assign a value to \$class2->var3 --\n";
$class2 = new myClass('x', 'y', 'z');
echo "\$key = var3:\n";
var_dump(array_key_exists('var3', $class2));
echo "\$class2:\n";
var_dump($class2);

echo "Done";
}
