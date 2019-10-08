<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

for ($i = 1; $i <= 5; ++$i)
{
    if (($i % 2) == 0)
        continue;
    echo "$i is odd\n";
}

for ($i = 1; $i <= 5; ++$i)
{
    $j = 20;
    while ($j > 0)
    {
        if ((($j * $i) % 2) == 0)
        {
            $j -= 3;
            continue;
        }
        echo ($j * $i)." is odd\n";
        $j -= 5;
    }
    echo "In for loop\n";
}

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; break;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    $break_after_switch = false;
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; $break_after_switch = true; break;
        case 30: echo "thirty"; break;
    }
    if ($break_after_switch) break;
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; continue;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    $continue_after_switch = false;
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; $continue_after_switch = true; continue;
        case 30: echo "thirty"; break;
    }
    if ($continue_after_switch) continue;
    echo "\nJust beyond the switch";
}
echo "\n----------\n";
}
