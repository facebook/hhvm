<?php
echo "Test 7: Transform To Uri";
include("prepare.inc");
$proc->importStylesheet($xsl);
print "\n";
$doc = $proc->transformToUri($dom, "file://".dirname(__FILE__)."/out.xml");
print file_get_contents(dirname(__FILE__)."/out.xml");
unlink(dirname(__FILE__)."/out.xml");
print "\n";

