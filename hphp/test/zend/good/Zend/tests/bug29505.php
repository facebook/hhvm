<?hh

class Test {
    public $empty = vec[];
    public $three = dict[0 => 1, "b" => "c", 3 => vec[]];
}
<<__EntryPoint>> function main(): void {
var_dump(get_class_vars('Test'));

echo "===DONE===\n";
}
