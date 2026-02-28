<?hh

<<__EntryPoint>>
function test_disable_ini_zend_compat_entrypoint() :mixed{
  var_dump(ini_get('post_max_size'));
  var_dump(ini_get('upload_max_filesize'));
  var_dump(ini_get('always_populate_raw_post_data'));
}
