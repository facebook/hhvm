<?hh
function test ($b) :mixed{
    $b++;
    return($b);
}
<<__EntryPoint>> function main(): void {
$a = test(1);
echo $a;
}
