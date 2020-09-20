<?hh

class foobar extends APCIterator {
  public function __construct() {}
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(E_ALL & ~E_USER_NOTICE & ~E_NOTICE);
  $obj = new foobar;
  var_dump(
    $obj->rewind(),
    $obj->current(),
    $obj->key(),
    $obj->next(),
    $obj->valid(),
    $obj->getTotalHits(),
    $obj->getTotalSize(),
    $obj->getTotalCount(),
    apc_delete($obj)
  );
}
