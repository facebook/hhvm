<?hh

// This example is taken from the PHP reference docs for the
// preg_replace_callback_array() function, with modification to use
// an array as the subject, etc.
<<__EntryPoint>>
function main_preg_replace_callback_array_with_array() :mixed{
$subject = vec['Aaafaaba BbaCZbx', '34wad3423 basd3z34qwe &&$#7723ABaac',
                 'kjklsdfjkldsiu3dklj12kl dsf;s4a;as bas'];
$count = 1000; // This should be reset to 0 when replacement starts.
$ret = preg_replace_callback_array(
    dict[
        '~[a]+~i' => function ($match) {},
        '~[b]+~i' => function ($match) {}
    ],
    $subject, -1, inout $count
);

var_dump($subject);
var_dump($ret);
var_dump($count);

// Let's try with just one element in the array

$subject = vec['kjklsdfjkldsiu3dklj12kl dsf;s4a;as bas'];
$count = -1; // This should be reset to 0 when replacement starts.
$ret = preg_replace_callback_array(
    dict[
        '~[a]+~i' => function ($match) {},
        '~[b]+~i' => function ($match) {}
    ],
    $subject, -1, inout $count
);

var_dump($subject);
var_dump($ret);
var_dump($count);
}
