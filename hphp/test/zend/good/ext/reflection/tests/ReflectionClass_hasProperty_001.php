<?hh
class pubf {
    public $a;
    static public $s;
}
class subpubf extends pubf {
}

class protf {
    protected $a;
    static protected $s;
}
class subprotf extends protf {
}

class privf {
    private $a;
    static protected $s;
}
class subprivf extends privf  {
}
<<__EntryPoint>> function main(): void {
$classes = vec["pubf", "subpubf", "protf", "subprotf",
                 "privf", "subprivf"];
foreach($classes as $class) {
    echo "Reflecting on class $class: \n";
    $rc = new ReflectionClass($class);
    echo "  --> Check for s: ";
    var_dump($rc->hasProperty("s"));
    echo "  --> Check for a: ";
    var_dump($rc->hasProperty("a"));
    echo "  --> Check for A: ";
    var_dump($rc->hasProperty("A"));
    echo "  --> Check for doesntExist: ";
    var_dump($rc->hasProperty("doesntExist"));
}
}
