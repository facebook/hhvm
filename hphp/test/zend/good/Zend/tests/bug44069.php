<?hh <<__EntryPoint>> function main(): void {
$array = vec[];
$newstring = "";
$string = str_repeat('This is a teststring.', 50);
for($i = 1; $i <= 2000; $i++)
{
//    $newstring .= $string; //This uses an expected amount of mem.
    $newstring = $newstring . $string; //This uses very much mem.

    for($j = 1; $j <= 10; $j++)
    {
        $array[] = 'test';
    }
}
echo "ok\n";
}
