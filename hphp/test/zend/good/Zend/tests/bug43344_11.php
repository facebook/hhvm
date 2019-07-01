<?hh
function f($a=namespace\bar) {
    return $a;
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
