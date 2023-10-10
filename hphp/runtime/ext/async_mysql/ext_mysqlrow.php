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

interface MysqlRow extends
  Countable,
  IteratorAggregate<mixed>,
  \HH\KeyedTraversable<string, mixed> {
  public function at(mixed $field): mixed;
  public function getFieldAsInt(mixed $field): int;
  public function getFieldAsDouble(mixed $field): float;
  public function getFieldAsString(mixed $field): string;
  public function fieldType(mixed $field): int;
  public function isNull(mixed $field): bool;
  public function count()[]: int;
  public function getIterator(): \HH\KeyedIterator<string, mixed>;
}
