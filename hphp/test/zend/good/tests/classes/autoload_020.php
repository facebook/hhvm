<?hh
function __autoload($name)
{
    echo "in autoload: $name\n";
}
<<__EntryPoint>> function main(): void {
var_dump(unserialize('O:1:"C":0:{}'));
}
