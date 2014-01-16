<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/* While the parameters are of type mixed, going to assume
   they are SplFileInfos for now. This will sort based on
   path, not including any appended file. Results will be
   similar to:

   /tmp/root/b34.txt
   /tmp/root/a.txt
   /tmp/root/z.txt
   /tmp/root/foo/we.txt
   /tmp/root/foo/a.txt
   /tmp/root/waz/bing.txt

   where the ending files are not necessarily in order, but the containing
   directories are in order.
*/
class SortedIterator extends SplHeap {
  public function __construct(Iterator $iterator) {
    foreach ($iterator as $item) {
      $this->insert($item);
    }
  }
  public function compare(mixed $b, mixed $a): int {
    return strcmp($a->getPath(), $b->getPath());
  }
}
