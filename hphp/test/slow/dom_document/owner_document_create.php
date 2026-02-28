<?hh

<<__EntryPoint>>
function main_owner_document_create() :mixed{
$dom = new DOMDocument();
var_dump($dom->createComment('Foo')->ownerDocument is DOMDocument);
var_dump($dom->importNode($dom->createComment('Foo'))->ownerDocument is DOMDocument);
}
