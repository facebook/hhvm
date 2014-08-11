<?php
$imp = new DOMImplementation();
$doctype = $imp->createDocumentType("html", 
	"-//W3C//DTD XHTML 1.0 Strict//EN", 
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
$doc = $imp->createDocument(null, 'html', $doctype);
echo $doc->saveHTML();
?>
