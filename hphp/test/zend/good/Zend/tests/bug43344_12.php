<?hh
function f($a=array(namespace\bar)) {
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
