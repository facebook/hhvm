<?hh
function __autoload($name) {
    echo "$name\n";
}
<<__EntryPoint>> function main(): void {
$a = "../BUG";
$x = new $a;
echo "BUG\n";
}
