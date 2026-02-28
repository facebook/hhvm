<?hh
<<__EntryPoint>> function main(): void {
$array = vec['foo', 'bar', 'baz'];
$iterator = new ArrayIterator($array);
$regexIterator = new RegexIterator($iterator, "/f/");

var_dump($regexIterator->getMode() === RegexIterator::MATCH);

$regexIterator->setMode(RegexIterator::MATCH);
var_dump($regexIterator->getMode() === RegexIterator::MATCH);

$regexIterator->setMode(RegexIterator::GET_MATCH);
var_dump($regexIterator->getMode() === RegexIterator::GET_MATCH);

$regexIterator->setMode(RegexIterator::ALL_MATCHES);
var_dump($regexIterator->getMode() === RegexIterator::ALL_MATCHES);

$regexIterator->setMode(RegexIterator::SPLIT);
var_dump($regexIterator->getMode() === RegexIterator::SPLIT);
}
