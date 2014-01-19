<?php

class books extends domDocument {
  function addBook($title, $author) {
    $titleElement = $this->createElement('title');
    $titleElement->appendChild($this->createTextNode($title));
    $authorElement = $this->createElement('author');
    $authorElement->appendChild($this->createTextNode($author));
    $bookElement = $this->createElement('book');
    $bookElement->appendChild($titleElement);
    $bookElement->appendChild($authorElement);
    $this->documentElement->appendChild($bookElement);
  }
}

$dom = new books;

$xml = <<<EOM
<?xml version='1.0' ?>
<books>
 <book>
  <title>The Grapes of Wrath</title>
  <author>John Steinbeck</author>
 </book> <book>
  <title>The Pearl</title>  <author>John Steinbeck</author>
 </book></books>
EOM;

$dom->loadXML($xml);
$dom->addBook('PHP de Luxe', 'Richard Samar, Christian Stocker');
print $dom->saveXML();
