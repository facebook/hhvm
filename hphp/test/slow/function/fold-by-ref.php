<?hh

function main() :mixed{
  $unpack = unpack("l", pack("l", hexdec("ff3a68be")));
  foreach ($unpack as $v) { return $v; }
}


<<__EntryPoint>>
function main_fold_by_ref() :mixed{
set_error_handler(function() {
    var_dump("Oops");
  });

var_dump(main());
}
