<?hh

function main() :mixed{
  $ch = curl_init();
  var_dump(is_resource($ch));
  curl_close($ch);
  var_dump(is_resource($ch));

  $ch = curl_multi_init();
  var_dump(is_resource($ch));
  curl_multi_close($ch);
  var_dump(is_resource($ch));
}


<<__EntryPoint>>
function main_handle_not_resource_after_close() :mixed{
main();
}
