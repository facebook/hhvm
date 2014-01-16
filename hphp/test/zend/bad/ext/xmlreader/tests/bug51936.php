<?php
echo "Test\n";

$xmlreader = new XMLReader();
$xmlreader->xml("<a><b/></a>");

$xmlreader->next();
$xmlreader2 = clone $xmlreader;
$xmlreader2->next();
?>
Done