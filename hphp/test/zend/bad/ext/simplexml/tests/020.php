<?php

$doc = simplexml_load_string('<root><name attr="foo">bar</name></root>');
print $doc->name["attr"];
print "\n";
if ($doc->name["attr"] == "foo") {
         print "Works\n";
} else {
         print "Error\n";
}
?>
===DONE===