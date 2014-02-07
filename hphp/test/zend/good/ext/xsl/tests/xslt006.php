<?php
echo "Test 6: Transform To Doc";
include("prepare.inc");
$proc->importStylesheet($xsl);
print "\n";
$doc = $proc->transformToDoc($dom);
print $doc->saveXML();
print "\n";

