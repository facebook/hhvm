<?php

class test implements Traversable {
}

$obj = new test;

foreach($obj as $v);

print "Done\n";
/* the error doesn't show the filename but 'Unknown' */
?>