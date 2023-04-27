<?hh
<<__EntryPoint>> function main(): void {
//correct offset
$dom = new DOMDocument();
$comment = $dom->createComment('test-comment');
$comment->insertData(4,'-inserted');
$dom->appendChild($comment);
echo $dom->saveXML();
}
