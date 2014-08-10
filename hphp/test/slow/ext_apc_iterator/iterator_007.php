<?php
class foobar extends APCIterator {
  public function __construct() {}
}
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
?>
