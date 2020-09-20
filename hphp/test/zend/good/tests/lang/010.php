<?hh
function test ($b) {
    $b++;
    return($b);
}
<<__EntryPoint>> function main(): void {
$a = test(1);
echo $a;
}
