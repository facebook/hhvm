<?hh

class test {
    static public $ar = varray[];
}
<<__EntryPoint>> function main(): void {
var_dump(test::$ar);

test::$ar[] = 1;

var_dump(test::$ar);

echo "Done\n";
}
