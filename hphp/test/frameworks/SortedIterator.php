<?hh

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
class SortedIterator<T> extends SplHeap<T> {
  public function __construct(Iterator<T> $iterator) {
    parent::__construct();
    foreach ($iterator as $item) {
      $this->insert($item);
    }
  }

  public function compare(T $b, T $a): int {
    assert($a instanceof SplFileInfo);
    assert($b instanceof SplFileInfo);
    return strcmp($a->getPath(), $b->getPath());
  }
}
