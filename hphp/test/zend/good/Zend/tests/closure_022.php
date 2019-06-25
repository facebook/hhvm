<?hh <<__EntryPoint>> function main(): void {
$a = 0;
$foo = function() use ($a) {
};
$foo->a = 1;
}
