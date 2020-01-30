<?hh


<<__EntryPoint>>
function main_recursive_caching_iterator_param1() {
try {
  new RecursiveCachingIterator(new ArrayIterator(varray[]));
} catch (InvalidArgumentException $e) {
  echo $e->getMessage() . "\n";
}

new RecursiveCachingIterator(Vector{});
}
