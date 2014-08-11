<?php
$s = simplexml_load_file(dirname(__FILE__)."/book.xml");
if(!$s) {
  echo "Error while loading the document\n";
  exit;
}
$dom = dom_import_simplexml($s);
print $dom->ownerDocument->saveXML();
?>
