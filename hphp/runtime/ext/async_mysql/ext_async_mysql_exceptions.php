<?hh
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

class AsyncMysqlException extends Exception {
  private AsyncMysqlErrorResult $result;
  public function __construct(AsyncMysqlErrorResult $result) {
    $this->result = $result;

    $this->message =
      'Error executing AsyncMysql operation: '.$result->failureType();
    if  ($this->mysqlErrorCode() > 0) {
      $this->message = $this->message.' (mysql error: '.$this->mysqlErrorCode().
        ': '.$this->mysqlErrorString().')';
    }
  }

  public function mysqlErrorCode() {
    return $this->result->mysql_errno();
  }

  public function mysqlErrorString() {
    return $this->result->mysql_error();
  }

  public function timedOut() {
    return $this->result->failureType() === "TimedOut";
  }

  public function failed() {
    return $this->result->failureType() === "Failed";
  }

  public function getResult() {
    return $this->result;
  }
}

class AsyncMysqlConnectException extends AsyncMysqlException { }

class AsyncMysqlQueryException extends AsyncMysqlException { }
