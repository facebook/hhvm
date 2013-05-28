<?php

$xml = <<< EOXML
<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE courses [
<!ELEMENT courses (course+)>
<!ELEMENT course (title, description, temp*)>
<!ATTLIST course cid ID #REQUIRED>
<!ELEMENT title (#PCDATA)>
<!ELEMENT description (#PCDATA)>
<!ELEMENT temp (#PCDATA)>
<!ATTLIST temp vid ID #REQUIRED>
<!ENTITY test 'http://www.hpl.hp.com/semweb/2003/query_tester#'>
<!ENTITY rdf  'http://www.w3.org/1999/02/22-rdf-syntax-ns#'>
<!NOTATION GIF PUBLIC "-" "image/gif">
<!ENTITY myimage PUBLIC "-" "mypicture.gif" NDATA GIF>
]>
<courses>
   <course cid="c1">
      <title>Basic Languages</title>
      <description>Introduction to Languages</description>
   </course>
   <course cid="c6">
      <title>French I</title>
      <description>Introduction to French</description>
      <temp vid="c7">
      </temp>
   </course>
</courses>
EOXML;

$dom = new DOMDocument();
$dom->loadXML($xml);

$dtd = $dom->doctype;

/* Notation Tests */
$nots = $dtd->notations;

$length = $nots->length;
var_dump($length);
echo "Length: ".$length."\n";

foreach ($nots AS $key=>$node) {
  echo "Key $key: ".$node->nodeName." (".
       $node->systemId.") (".$node->publicId.")\n";
}
print "
";
for($x=0;
 $x < $length;
 $x++) {
  echo "Index $x: ".$nots->item($x)->nodeName." (".
       $nots->item($x)->systemId.") (".$nots->item($x)->publicId.")\n";
}

echo "\n";
$node = $nots->getNamedItem('xxx');
var_dump($node);

echo "\n";
/* Entity Decl Tests */
$ents = $dtd->entities;

$length = $ents->length;
echo "Length: ".$length."\n";

$a = array();
foreach ($ents AS $key=>$node) {
  $a[$key] = $node;
}
ksort($a);

foreach ($a as $key => $node) {
 echo "Key: $key Name: ".$node->nodeName."\n";
}
echo "\n";

$node = $ents->getNamedItem('xxx');
var_dump($node);
