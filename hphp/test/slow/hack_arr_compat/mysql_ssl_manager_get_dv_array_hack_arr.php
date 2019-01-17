<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_mysql_ssl_manager_get_typehint_notices() {
  mysql_ssl_manager_get(
    darray[
      'endpoint_sys_tier' => 'dbs.60006',
      'db_user' => 'scriptro:sys.unittestdb',
    ],
  );
  echo "Done\n";
}
