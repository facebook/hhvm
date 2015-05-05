<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main($o) {
  var_dump(array_key_exists(null, $o));
}
main(new stdclass);
