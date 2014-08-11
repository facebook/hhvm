<?php
$xml  = '<?xml version="1.0" encoding="UTF-8" ?>';
$xml .= '<!DOCTYPE chapter SYSTEM "http://www.xmlwriter.net/logo.gif">';
$xml .= '<chapter>1</chapter>';
$doc = new DOMDocument();
$doc->loadXML($xml);
$doctype = $doc->doctype;
var_dump($doctype->publicId);
?>
