<?php 

$xml =<<<EOF
<?xml version='1.0'?>
<!DOCTYPE sxe SYSTEM "notfound.dtd">
<sxe id="elem1">
 <elem1 attr1='first'>
  <!-- comment -->
  <elem2>
   <elem3>
    <elem4>
     <?test processing instruction ?>
    </elem4>
   </elem3>
  </elem2>
 </elem1>
</sxe>
EOF;

$sxe = simplexml_load_string($xml);

var_dump($sxe->xpath("elem1/elem2/elem3/elem4"));
//valid expression 
var_dump($sxe->xpath("***"));
//invalid expression 
var_dump($sxe->xpath("**"));
?>