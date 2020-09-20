<?hh

class myIterator implements Iterator {

function current (){}
function key ( ){}
function next ( ){}
function rewind ( ){}
function valid ( ){}


}

class TestRegexIterator extends RegexIterator{}
<<__EntryPoint>> function main(): void {
$rege = '/^a/';


$r = new TestRegexIterator(new myIterator, $rege);

$r->setPregFlags(PREG_OFFSET_CAPTURE);

echo $r->getPregFlags();
}
