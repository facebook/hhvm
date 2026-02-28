<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

///*
// Two non-numeric strings

$oper1 = vec["", "a", "aa", "a0", "aA"];
$oper2 = vec["", "ab", "abc", "A", "AB"];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} >        {$e2}  result: "; var_dump($e1 > $e2);
        echo "{$e2} <=       {$e1}  result: "; var_dump($e2 <= $e1);
        echo "---\n";
        echo "{$e1} >=       {$e2}  result: "; var_dump($e1 >= $e2);
        echo "{$e2} <        {$e1}  result: "; var_dump($e2 < $e1);
        echo "---\n";
        echo "{$e1} <        {$e2}  result: "; var_dump($e1 < $e2);
        echo "{$e2} >=       {$e1}  result: "; var_dump($e2 >= $e1);
        echo "---\n";
        echo "{$e1} <=       {$e2}  result: "; var_dump($e1 <= $e2);
        echo "{$e2} >        {$e1}  result: "; var_dump($e2 > $e1);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
}
