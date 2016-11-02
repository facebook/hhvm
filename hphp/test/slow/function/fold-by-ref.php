<?php

set_error_handler(function() {
    var_dump("Oops");
  });

function main() {
  return reset(unpack("l", pack("l", hexdec("ff3a68be"))));
}

var_dump(main());
