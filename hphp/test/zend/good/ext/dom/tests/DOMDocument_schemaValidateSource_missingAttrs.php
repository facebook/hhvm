<?php

$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book-attr.xml");

$xsd = file_get_contents(dirname(__FILE__)."/book.xsd");

$doc->schemaValidateSource($xsd);

foreach ($doc->getElementsByTagName('book') as $book) {
    var_dump($book->getAttribute('is-hardback'));
}

?>
