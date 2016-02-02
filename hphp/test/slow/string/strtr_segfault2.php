<?php
// at least 64 patterns required to trigger the WuManberReplacement algorithm
// One of the pattern needs to be longer than one byte
$subst = range( 0, 65 );

// source string must not have fewer bytes then the smallest pattern above
echo strtr( "ab", $subst ), "\n";
