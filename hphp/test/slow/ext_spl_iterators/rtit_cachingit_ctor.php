<?hh


<<__EntryPoint>>
function main_rtit_cachingit_ctor() {
try {
  new RecursiveTreeIterator(new ArrayIterator(varray[]));
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
  // RecursiveCachingIterator expects param1 to be recit
}
}
