<?hh
function __autoload($name)
{
    echo "In autoload: ";
    var_dump($name);
}
<<__EntryPoint>> function main(): void {
$a = new stdClass;
var_dump($a is UndefC);
}
