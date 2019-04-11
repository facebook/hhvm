<?php
$a = array(
    'a' => array(
        'A', 'B', 'C', 'D',
    ),
    'b' => array(
        'AA', 'BB', 'CC', 'DD',
    ),
);

// Set the pointer of $a to 'b' and the pointer of 'b' to 'CC'
reset(&$a);
next(&$a);
$ab = $a['b'];
next(&$ab);
next(&$ab);
next(&$ab);

var_dump(key(&$ab));
foreach($a as $k => $d)
{
}
// Alternatively $c = $a; and foreachloop removal will cause identical results.
var_dump(key(&$ab));
