<?hh
function __autoload($name)
{
    echo "In autoload: ";
    var_dump($name);
}
<<__EntryPoint>> function main() {
try {
    new ReflectionClass("UndefC");
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
