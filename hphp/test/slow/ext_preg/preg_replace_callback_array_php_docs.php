<?php
// This example is taken from the PHP reference docs for the
// preg_replace_callback_array() function, with slight modification.
$subject = 'Aaafaaba BbaCZbx';
$count = 0;
$ret = preg_replace_callback_array(
    [
        '~[a]+~i' => function ($match) {},
        '~[b]+~i' => function ($match) {}
    ],
    $subject, -1, $count
);

var_dump($ret);
var_dump($count);
