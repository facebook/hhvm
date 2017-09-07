<?php

require_once('apc_fetch_object_fast_init.inc');

if (!apc_exists('my_key')) {
  $c = new C;
  $c->lol = 'wat';
  var_dump(apc_store('my_key', $c));
}
var_dump(apc_fetch('my_key'));
