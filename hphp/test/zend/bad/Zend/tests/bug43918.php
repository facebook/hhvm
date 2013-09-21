<?php
$xmlstr = <<<XML
<?xml version='1.0' standalone='yes'?>
<movies>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
 <movie>
  <title>TEST</title>
 </movie>
</movies>
XML;

$Array = array( );
for( $XX = 0; $XX < 2000; ++$XX )
{
 $Array[] = $xml = new SimpleXMLElement($xmlstr);
}

gc_collect_cycles( );
echo "ok\n";
?>