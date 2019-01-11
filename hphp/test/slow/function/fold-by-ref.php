<?php

function main() {
  return reset(&unpack("l", pack("l", hexdec("ff3a68be"))));
}


<<__EntryPoint>>
function main_fold_by_ref() {
set_error_handler(function() {
    var_dump("Oops");
  });

var_dump(main());
}
