<?hh
function autoloadA($name) {
    echo "A -> $name\n";
}
function autoloadB($name) {
    echo "B -> $name\n";
}
function autoloadC($name) {
    echo "C -> $name\n";
    include 'spl_autoload_010.inc';
}
<<__EntryPoint>> function main(): void {
spl_autoload_register('autoloadA');
spl_autoload_register('autoloadB', true, true);
spl_autoload_register('autoloadC');

new C();
echo "===DONE===\n";
}
