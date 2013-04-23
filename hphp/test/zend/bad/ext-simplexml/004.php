<?php 

$sxe = simplexml_load_string(<<<EOF
<?xml version='1.0'?>
<!DOCTYPE sxe SYSTEM "notfound.dtd">
<sxe id="elem1">
 Plain text.
 <elem1 attr1='first'>
  <!-- comment -->
  <elem2>
   <![CDATA[CDATA block]]>
   <elem3>
    <elem4>
     <?test processing instruction ?>
    </elem4>
   </elem3>
  </elem2>
 </elem1>
</sxe>
EOF
);

var_dump($sxe);

$elem1 = $sxe->elem1;
$elem2 = $elem1->elem2;
var_dump(trim((string)$elem2));

?>
===DONE===