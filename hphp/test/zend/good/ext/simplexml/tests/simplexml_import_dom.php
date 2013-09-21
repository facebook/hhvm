<?php 
$dom = new domDocument;
$dom->load(dirname(__FILE__)."/book.xml");
if(!$dom) {
  echo "Error while parsing the document\n";
  exit;
}
$s = simplexml_import_dom($dom);
$books = $s->book;
foreach ($books as $book) {
	echo "{$book->title} was written by {$book->author}\n";
}
?>