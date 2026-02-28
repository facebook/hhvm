<?hh
function show_eq($x,$y) :mixed{
    echo "$x ". ($x==$y ? "==" : "!=") ." $y\n";
}
<<__EntryPoint>> function main(): void {
$value = 0x7FFFFFFF; //2147483647;
show_eq("$value", json_encode($value));
$value++;
show_eq("$value", json_encode($value));
}
