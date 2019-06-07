<?hh <<__EntryPoint>> function main() {
$a = 0;
$foo = function() use ($a) {
};
$foo->a = 1;
}
