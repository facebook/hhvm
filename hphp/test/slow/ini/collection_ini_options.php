<?hh

function sort_opt($a) :mixed{
  ksort(inout $a);
  var_dump($a);
}

<<__EntryPoint>>
function main_collection_ini_options() :mixed{
var_dump(ini_get("hhvm.server.allowed_directories"));
sort_opt(ini_get("hhvm.static_file.extensions"));
var_dump(ini_get("hhvm.server.forbidden_file_extensions"));
var_dump(ini_get("hhvm.server.high_priority_end_points"));
}
