<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
define('SQLITE3_ASSOC', 0);
define('SQLITE3_NUM', 0);
define('SQLITE3_BOTH', 0);
define('SQLITE3_INTEGER', 0);
define('SQLITE3_FLOAT', 0);
define('SQLITE3_TEXT', 0);
define('SQLITE3_BLOB', 0);
define('SQLITE3_NULL', 0);
define('SQLITE3_OPEN_READONLY', 0);
define('SQLITE3_OPEN_READWRITE', 0);
define('SQLITE3_OPEN_CREATE', 0);
class SQLite3 {
  public function __construct($filename, $flags = null, $encryption_key = null) { }
  public function open($filename, $flags = null, $encryption_key = null) { }
  public function busytimeout($msecs) { }
  public function close() { }
  public function exec($sql) { }
  public function version() { }
  public function lastinsertrowid() { }
  public function lasterrorcode() { }
  public function lasterrormsg() { }
  public function loadextension($extension) { }
  public function changes() { }
  public function escapestring($sql) { }
  public function prepare($sql) { }
  public function query($sql) { }
  public function querysingle($sql, $entire_row = false) { }
  public function createfunction($name, $callback, $argcount = -1) { }
  public function createaggregate($name, $step, $final, $argcount = -1) { }
  public function openblob($table, $column, $rowid, $dbname = null) { }
}
class SQLite3Stmt {
  public function __construct($dbobject, $statement) { }
  public function paramcount() { }
  public function close() { }
  public function reset() { }
  public function clear() { }
  public function bindparam($name, &$parameter, $type = SQLITE3_TEXT) { }
  public function bindvalue($name, $parameter, $type = SQLITE3_TEXT) { }
  public function execute() { }
}
class SQLite3Result {
  public function __construct() { }
  public function numcolumns() { }
  public function columnname($column) { }
  public function columntype($column) { }
  public function fetcharray($mode = SQLITE3_BOTH) { }
  public function reset() { }
  public function finalize() { }
}
