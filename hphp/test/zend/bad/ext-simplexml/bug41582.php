<?php

$xml = new SimpleXMLElement('<?xml version="1.0" standalone="yes"?>
<collection></collection>');

$xml->movie[]->characters->character[0]->name = 'Miss Coder';

echo($xml->asXml());

?>
===DONE===