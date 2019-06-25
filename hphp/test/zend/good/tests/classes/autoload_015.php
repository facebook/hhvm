<?hh
function __autoload($name)
{
    echo "In autoload: ";
    var_dump($name);
}
<<__EntryPoint>> function main(): void {
try {
    new ReflectionProperty('UndefC', 'p');
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
