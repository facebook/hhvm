<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int SQLITE3_ASSOC = 0;
const int SQLITE3_NUM = 0;
const int SQLITE3_BOTH = 0;
const int SQLITE3_INTEGER = 0;
const int SQLITE3_FLOAT = 0;
const int SQLITE3_TEXT = 0;
const int SQLITE3_BLOB = 0;
const int SQLITE3_NULL = 0;
const int SQLITE3_OPEN_READONLY = 0;
const int SQLITE3_OPEN_READWRITE = 0;
const int SQLITE3_OPEN_CREATE = 0;

class SQLite3 {
  public function __construct(string $filename, int $flags = SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE, $encryption_key = null);
  public function open(string $filename, int $flags = SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE, $encryption_key = null);
  public function busytimeout(int $msecs);
  public function close();
  public function exec(string $sql);
  public static function version();
  public function lastinsertrowid();
  public function lasterrorcode();
  public function lasterrormsg();
  public function loadextension(string $extension);
  public function changes();
  public static function escapestring(string $sql);
  public function prepare(string $sql);
  public function query(string $sql);
  public function querysingle(string $sql, bool $entire_row = false);
  public function createfunction(string $name, $callback, int $argcount = -1);
  public function createaggregate(string $name, $step, $final, int $argcount = -1);
  public function openblob(string $table, string $column, int $rowid, $dbname = null);
}

class SQLite3Stmt {
  public function __construct($dbobject, string $statement);
  public function paramcount();
  public function close();
  public function reset();
  public function clear();
  public function bindvalue($name, $parameter, int $type = SQLITE3_TEXT);
  public function execute();
}

class SQLite3Result {
  public function __construct();
  public function numcolumns();
  public function columnname(int $column);
  public function columntype(int $column);
  public function fetcharray(int $mode = SQLITE3_BOTH);
  public function reset();
  public function finalize();
}
