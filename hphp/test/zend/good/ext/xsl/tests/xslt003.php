<?php
echo "Test 3: Using Parameters";
include("prepare.inc");
$proc->importStylesheet($xsl);
$proc->setParameter( "", "foo","hello world");
print "\n";
print $proc->transformToXml($dom);
print "\n";

