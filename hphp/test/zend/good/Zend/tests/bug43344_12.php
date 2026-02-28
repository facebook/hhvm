<?hh
function f($a=vec[namespace\bar]) :mixed{
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
