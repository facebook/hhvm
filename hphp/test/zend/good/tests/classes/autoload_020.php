<?hh
function __autoload($name)
{
    echo "in autoload: $name\n";
}
<<__EntryPoint>> function main() {
var_dump(unserialize('O:1:"C":0:{}'));
}
