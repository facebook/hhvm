<?hh

// https://github.com/php/php-src/pull/865


<<__EntryPoint>>
function main_recursive_regex_iterator_get_children_flags() {
$arr = new RecursiveArrayIterator(varray[varray['test2', 'test3']]);
$regex = new RecursiveRegexIterator($arr, '/^test/',
           RecursiveRegexIterator::ALL_MATCHES, 1, 2);

$child = $regex->getChildren();
var_dump(get_class($child),
         $child->getMode(),
         $child->getFlags(),
         $child->getPregFlags()
         );
}
