<?php
function autoloadA($name) {
    echo "A -> $name\n";
}
function autoloadB($name) {
    echo "B -> $name\n";
}
function autoloadC($name) {
    echo "C -> $name\n";
    class C{}
}

spl_autoload_register('autoloadA');
spl_autoload_register('autoloadB', true, true);
spl_autoload_register('autoloadC');

new C;
?>
===DONE===
<?php exit(0); ?>