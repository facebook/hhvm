<?hh
/**
 * @author Joshua Thijssen <jthijssen+php@noxlogic.nl>
 */
<<__EntryPoint>> function main(): void {
$it = new \ArrayIterator(vec["foo", "bar", "baz"]);
$it2 = new \RegexIterator($it, "/^ba/", \RegexIterator::MATCH);
print_r(iterator_to_array($it2));
$it2 = new \RegexIterator($it, "/^ba/", \RegexIterator::MATCH, \RegexIterator::INVERT_MATCH);
print_r(iterator_to_array($it2));

$it = new \ArrayIterator(dict["foo" => 1, "bar" => 2, "baz" => 3]);
$it2 = new \RegexIterator($it, "/^ba/", \RegexIterator::MATCH, \RegexIterator::USE_KEY);
print_r(iterator_to_array($it2));
$it2 = new \RegexIterator($it, "/^ba/", \RegexIterator::MATCH, \RegexIterator::USE_KEY | \RegexIterator::INVERT_MATCH);
print_r(iterator_to_array($it2));
}
