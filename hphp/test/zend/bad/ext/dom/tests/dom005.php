<?php
$dom = new domdocument;
$dom->loadHTMLFile(dirname(__FILE__)."/test.html");
print  "--- save as XML\n";

print adjustDoctype($dom->saveXML());
print  "--- save as HTML\n";

print adjustDoctype($dom->saveHTML());

function adjustDoctype($xml) {
    return str_replace(array("DOCTYPE HTML",'<p>','</p>'),array("DOCTYPE html",'',''),$xml);
}

