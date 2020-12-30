<?hh

<<__EntryPoint>>
function main() {
  $flags = TAG_PROVENANCE_HERE_MUTATE_COLLECTIONS;

  $collection = Vector { varray[] };
  print(HH\get_provenance($collection[0])."\n");
  $collection = HH\tag_provenance_here($collection);
  print(HH\get_provenance($collection[0])."\n");
  $collection = HH\tag_provenance_here($collection, $flags);
  print(HH\get_provenance($collection[0])."\n");

  // Same as above, except that we also test the fact that tag_provenance_here
  // on collection inputs does an in-place mutation (unlike on array-likes).
  $collection = Vector { varray[] };
  print(HH\get_provenance($collection[0])."\n");
  HH\tag_provenance_here($collection);
  print(HH\get_provenance($collection[0])."\n");
  HH\tag_provenance_here($collection, $flags);
  print(HH\get_provenance($collection[0])."\n");
}
