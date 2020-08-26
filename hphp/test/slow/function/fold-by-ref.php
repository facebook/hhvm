<?hh

function main() {
  $unpack = unpack("l", pack("l", hexdec("ff3a68be")));
  foreach ($unpack as $v) { return $v; }
}


<<__EntryPoint>>
function main_fold_by_ref() {
set_error_handler(function() {
    var_dump("Oops");
  });

var_dump(main());
}
