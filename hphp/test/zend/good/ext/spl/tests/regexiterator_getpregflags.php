<?hh

class myIterator implements Iterator {

function current ():mixed{}
function key ( ):mixed{}
function next ( ):mixed{}
function rewind ( ):mixed{}
function valid ( ):mixed{}


}

class TestRegexIterator extends RegexIterator{}
<<__EntryPoint>> function main(): void {
$rege = '/^a/';


$r = new TestRegexIterator(new myIterator, $rege);

$r->setPregFlags(PREG_OFFSET_CAPTURE);

echo is_long($r->getPregFlags());
}
