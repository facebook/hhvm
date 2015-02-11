<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

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
            continue 1;
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
        case 20: echo "twenty"; break 1;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; break 2;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; continue 1;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";

for ($i = 10; $i <= 40; $i +=10)
{
    echo "\n\$i = $i: ";
    switch($i)
    {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; continue 2;
        case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
}
echo "\n----------\n";
