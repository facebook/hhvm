<?hh

class foo {
    protected $a = array();
}
<<__EntryPoint>> function main(): void {
$a = new foo;
$x = json_encode($a);

print_r($a);
}
