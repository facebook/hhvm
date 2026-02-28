<?hh
class pubf {
    public function f() :mixed{}
    static public function s() :mixed{}
}
class subpubf extends pubf {
}

class protf {
    protected function f() :mixed{}
    static protected function s() :mixed{}
}
class subprotf extends protf {
}

class privf {
    private function f() :mixed{}
    static private function s() :mixed{}
}
class subprivf extends privf  {
}
<<__EntryPoint>> function main(): void {
$classes = vec["pubf", "subpubf", "protf", "subprotf",
                 "privf", "subprivf"];
foreach($classes as $class) {
    echo "Reflecting on class $class: \n";
    $rc = new ReflectionClass($class);
    echo "  --> Check for f(): ";
    var_dump($rc->hasMethod("f"));
    echo "  --> Check for s(): ";
    var_dump($rc->hasMethod("s"));
    echo "  --> Check for F(): ";
    var_dump($rc->hasMethod("f"));
    echo "  --> Check for doesntExist(): ";
    var_dump($rc->hasMethod("doesntExist"));
}
}
