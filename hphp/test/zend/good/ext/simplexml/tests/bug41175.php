<?php
<<__EntryPoint>> function main() {
$xml = new SimpleXmlElement("<img></img>");
$xml->addAttribute("src", "foo");
$xml->addAttribute("alt", "");
echo $xml->asXML();

echo "===DONE===\n";
}
