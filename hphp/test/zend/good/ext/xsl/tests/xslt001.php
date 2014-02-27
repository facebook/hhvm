<?php
echo "Test 1: Transform To XML String";
include("prepare.inc");
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXml($dom);
print "\n";

