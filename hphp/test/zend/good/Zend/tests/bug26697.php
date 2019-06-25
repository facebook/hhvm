<?hh

function __autoload($name)
{
    echo __METHOD__ . "($name)\n";
    var_dump(class_exists('NotExistingClass'));
    echo __METHOD__ . "($name), done\n";
}
<<__EntryPoint>> function main(): void {
var_dump(class_exists('NotExistingClass'));

echo "===DONE===\n";
}
