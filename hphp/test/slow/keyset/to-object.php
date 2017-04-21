<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// Verify that when a keyset is converted to an object, it's dynamic properties
// is stored in a PHP array.

function to_object($ks) {
  $obj = (object)$ks;
  var_dump($obj);
  var_dump($obj->foobar);
  var_dump($obj->notthere);
  $obj->blah = 100;
  var_dump($obj->blah);
}

to_object(keyset['abc', 'foobar', 'def']);
