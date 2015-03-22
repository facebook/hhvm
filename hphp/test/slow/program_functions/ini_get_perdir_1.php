<?php
//
// Like ini_get_perdir_0.php,
// but HDF assignments are used in the .opts file
//
function dump_ini_setting($name) {
  $value = ini_get($name);
  printf("%55s %s\n", $name, $value);
}

dump_ini_setting("overwrite.hhvm.server.upload.upload_max_file_size");
dump_ini_setting(          "hhvm.server.upload.upload_max_file_size");
dump_ini_setting(                             "upload_max_filesize");

// That's right, the root words for the name are permuted in
// hhvm namespace from php namespace.
dump_ini_setting("hhvm.overwrite.server.max_post_size");
dump_ini_setting(          "hhvm.server.max_post_size");
dump_ini_setting(                      "post_max_size");

dump_ini_setting("hhvm.server.always_populate_raw_post_data");
dump_ini_setting(            "always_populate_raw_post_data");

// hhvm namespace doesn't expose auto_prepend_file
dump_ini_setting(            "auto_prepend_file");

// hhvm namespace doesn't expose auto_append_file
dump_ini_setting(            "auto_append_file");


//
// Attempts to ini_set these variables should fail,
// as they are all all PHP_INI_PERDIR
//
ini_set("upload_max_filesize", 77777777777);
ini_set("post_max_size", 888888888);
ini_set("always_populate_raw_post_data", false);
ini_set("auto_prepend_file", "///dev/null");
ini_set("auto_append_file", "///dev/null");

dump_ini_setting("upload_max_filesize");
dump_ini_setting("post_max_size");
dump_ini_setting("always_populate_raw_post_data");
dump_ini_setting("auto_prepend_file");
dump_ini_setting("auto_append_file");
