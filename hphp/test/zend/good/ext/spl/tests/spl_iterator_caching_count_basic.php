<?hh <<__EntryPoint>> function main(): void {
$i = new ArrayIterator(varray[1,1,1,1,1]);
$i = new CachingIterator($i,CachingIterator::FULL_CACHE);
foreach ($i as $value) {
  echo $i->count()."\n";
}
echo "===DONE===\n";
}
