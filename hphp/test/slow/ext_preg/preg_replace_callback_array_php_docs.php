<?hh

// This example is taken from the PHP reference docs for the
// preg_replace_callback_array() function, with slight modification.
<<__EntryPoint>>
function main_preg_replace_callback_array_php_docs() :mixed{
$subject = 'Aaafaaba BbaCZbx';
$count = 0;
$ret = preg_replace_callback_array(
    dict[
        '~[a]+~i' => function ($match) {},
        '~[b]+~i' => function ($match) {}
    ],
    $subject, -1, inout $count
);

var_dump($ret);
var_dump($count);
}
