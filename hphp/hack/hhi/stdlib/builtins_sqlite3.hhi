<?hh /* -*- php -*- */
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
  public function __construct(
    string $filename,
    int $flags = SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE,
    $encryption_key = null,
  );
  public function open(
    string $filename,
    int $flags = SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE,
    $encryption_key = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function busytimeout(int $msecs): HH\FIXME\MISSING_RETURN_TYPE;
  public function close(): HH\FIXME\MISSING_RETURN_TYPE;
  public function exec(string $sql): HH\FIXME\MISSING_RETURN_TYPE;
  public static function version(): HH\FIXME\MISSING_RETURN_TYPE;
  public function lastinsertrowid(): HH\FIXME\MISSING_RETURN_TYPE;
  public function lasterrorcode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function lasterrormsg(): HH\FIXME\MISSING_RETURN_TYPE;
  public function loadExtension(
    string $extension,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function changes(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function escapestring(
    string $sql,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function prepare(string $sql): HH\FIXME\MISSING_RETURN_TYPE;
  public function query(string $sql): HH\FIXME\MISSING_RETURN_TYPE;
  public function querysingle(
    string $sql,
    bool $entire_row = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createfunction(
    string $name,
    $callback,
    int $argcount = -1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createaggregate(
    string $name,
    $step,
    $final,
    int $argcount = -1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function openblob(
    string $table,
    string $column,
    int $rowid,
    $dbname = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

class SQLite3Stmt {
  public function __construct($dbobject, string $statement);
  public function paramcount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function close(): HH\FIXME\MISSING_RETURN_TYPE;
  public function reset(): HH\FIXME\MISSING_RETURN_TYPE;
  public function clear(): HH\FIXME\MISSING_RETURN_TYPE;
  public function bindvalue(
    $name,
    $parameter,
    int $type = SQLITE3_TEXT,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function execute(): HH\FIXME\MISSING_RETURN_TYPE;
}

class SQLite3Result {
  public function __construct();
  public function numcolumns(): HH\FIXME\MISSING_RETURN_TYPE;
  public function columnname(int $column): HH\FIXME\MISSING_RETURN_TYPE;
  public function columntype(int $column): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetcharray(
    int $mode = SQLITE3_BOTH,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function reset(): HH\FIXME\MISSING_RETURN_TYPE;
  public function finalize(): HH\FIXME\MISSING_RETURN_TYPE;
}
