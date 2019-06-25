<?hh
function __autoload($className) {
    print "$className\n";
    exit();
}

function foo($c = ok::constant) {
}
<<__EntryPoint>> function main(): void {
foo();
}
