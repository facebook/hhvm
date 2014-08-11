<?php
$rng = <<< RNG
<?xml version="1.0" encoding="UTF-8"?> 
<grammar ns="" xmlns="http://relaxng.org/ns/structure/1.0" 
  datatypeLibrary="http://www.w3.org/2001/XMLSchema-datatypes"> 
  <start> 
    <element name="apple"> 
      <element name="pear"> 
        <data type="NCName"/> 
      </element> 
    </element> 
  </start> 
</grammar>
RNG;

$bad_xml = <<< BAD_XML
<?xml version="1.0"?> 
<apple> 
  <pear>Pear</pear> 
  <pear>Pear</pear> 
</apple>
BAD_XML;

$doc = new DOMDocument();
$doc->loadXML($bad_xml);
$result = $doc->relaxNGValidateSource($rng);
var_dump($result);

?>
