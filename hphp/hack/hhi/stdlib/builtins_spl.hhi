<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function spl_classes();
<<__PHPStdLib>>
function spl_object_hash($obj);
<<__PHPStdLib>>
function hphp_object_pointer($obj);
<<__PHPStdLib>>
function hphp_get_this();
<<__PHPStdLib>>
function class_implements($obj, bool $autoload = true);
<<__PHPStdLib>>
function class_parents($obj, bool $autoload = true);
<<__PHPStdLib>>
function class_uses($obj, bool $autoload = true);
<<__PHPStdLib>>
function iterator_apply($obj, $func, $params = null);
<<__PHPStdLib>>
function iterator_count($obj);
<<__PHPStdLib>>
function iterator_to_array($obj, bool $use_keys = true);

class SplDoublyLinkedList<T> implements Iterator<T>, ArrayAccess<int, T>, Countable {
  public function bottom(): T;
  public function isEmpty(): bool;
  public function key(): int;
  public function pop(): T;
  public function prev(): void;
  public function push(T $val): void;
  public function serialize(): string;
  public function shift(): T;
  public function top(): T;
  public function unserialize(string $str): void;
  public function unshift(T $val): void;

  public function current(): T;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;

  public function offsetExists(/*int*/ $key): bool;
  public function offsetGet(/*int*/ $key): T;
  public function offsetSet(/*int*/ $key, T $val): void;
  public function offsetUnset(/*int*/ $key): void;

  public function count(): int;
}

class SplQueue<T> extends SplDoublyLinkedList<T> {
  public function __construct();
  public function dequeue(): T;
  public function enqueue(T $val): void;
}

class SplFileInfo {

  // Methods
  public function __construct(string $file_name);
  public function getATime(): int;
  public function getBasename(string $suffix = ''): string;
  public function getCTime(): int;
  public function getExtension(): string;
  public function getFileInfo(?string $class_name = null): this;
  public function getFilename(): string;
  public function getGroup(): int;
  public function getInode(): int;
  public function getLinkTarget(): string;
  public function getMTime(): int;
  public function getOwner(): int;
  public function getPath(): string;
  public function getPathInfo(?string $class_name = null): this;
  public function getPathname(): string;
  public function getPerms(): int;
  public function getRealPath(): string;
  public function getSize(): int;
  public function getType(): string;
  public function isDir(): bool;
  public function isExecutable(): bool;
  public function isFile(): bool;
  public function isLink(): bool;
  public function isReadable(): bool;
  public function isWritable(): bool;
  public function openFile(
    string $open_mode = "r",
    bool $use_include_path = false,
    ?resource $context = null,
  ): SplFileObject;
  public function setFileClass(string $class_name = "SplFileObject"): void;
  public function setInfoClass(string $class_name = "SplFileInfo"): void;
}

class SplFileObject extends SplFileInfo
  implements
    SeekableIterator<string>,
    RecursiveIterator<SplFileObject> {

  // Constants
  const int DROP_NEW_LINE = 1;
  const int READ_AHEAD = 2;
  const int SKIP_EMPTY = 4;
  const int READ_CSV = 8;

  // Methods
  public function __construct(
    string $filename,
    string $open_mode = "r",
    bool $use_include_path = false,
    ?resource $context = null,
  );
  /** @return string|array violates Iterator interface */
  public function current();
  public function eof(): bool;
  public function fflush(): bool;
  public function fgetc(): string;
  public function fgetcsv(
    string $delimiter = ",",
    string $enclosure = "\"",
    string $escape = "\\",
    /* HH_IGNORE_ERROR[2071] */
  ): varray;
  public function fgets();
  public function fgetss(?string $allowable_tags = null): string;
  public function flock(int $operation, inout mixed $wouldblock): bool;
  public function fpassthru(): int;
  public function fputcsv(
    /* HH_IGNORE_ERROR[2071] */
    varray $fields,
    string $delimiter = ",",
    string $enclosure = '"',
  ): int;
  public function fread(int $length): string;
  public function fscanf(string $format): varray;
  public function fseek(int $offset, int $whence = SEEK_SET): int;
  /* HH_IGNORE_ERROR[2071] */
  public function fstat(): darray;
  public function ftell(): int;
  public function ftruncate(int $size): bool;
  public function fwrite(string $str, int $length): int;
  /* HH_IGNORE_ERROR[2071] */
  public function getCsvControl(): varray;
  public function getFlags(): int;
  public function getMaxLineLen(): int;
  public function setCsvControl(
    string $delimiter = ",",
    string $enclosure = "\"",
    string $escape = "\\",
  ): void;
  /* (always) returns null -- violates RecursiveIterator interface */
  public function getChildren();
  public function hasChildren(): bool;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function seek(int $line_pos): void;
  public function setFlags(int $flags): void;
  public function setMaxLineLen(int $max_len): void;
  public function valid(): bool;
}

class SplTempFileObject extends SplFileObject
  implements
    SeekableIterator<string>,
    RecursiveIterator<SplTempFileObject> {

  // Methods
  public function __construct(?int $max_memory = null);
}

class DirectoryIterator extends SplFileInfo
  implements SeekableIterator<DirectoryIterator> {

  // Methods
  public function __construct(string $path);
  public function current(): DirectoryIterator;
  public function isDot(): bool;
  public function next(): void;
  public function rewind(): void;
  public function seek(int $position): void;
  public function valid(): bool;
}

class NoRewindIterator<T> extends IteratorIterator<T> {
}

class InfiniteIterator<T> extends IteratorIterator<T> {
}

class GlobIterator extends FilesystemIterator implements Countable {
  public function count() {}
}

class SplStack<T> extends SplDoublyLinkedList<T> {
}

class SplPriorityQueue<T> implements Iterator<T>, Countable {
  const int EXTR_BOTH = 3;
  const int EXTR_PRIORITY = 2;
  const int EXTR_DATA = 1;
  public function compare($a, $b) {}
  public function count() {}
  public function current() {}
  public function extract() {}
  public function insert($value, $priority) {}
  public function isEmpty() {}
  public function key() {}
  public function next() {}
  public function recoverFromCorruption() {}
  public function rewind() {}
  public function setExtractFlags($flags) {}
  public function top() {}
  public function valid() {}
}

interface SplObserver {
  public function update(SplSubject $SplSubject) {}
}

interface SplSubject {
  public function attach(SplObserver $SplObserver) {}
  public function detach(SplObserver $SplObserver) {}
  public function notify() {}
}

class SplType {
  public function __construct($initial_value, $strict) {}
}

class SplInt extends SplType {
}

class SplFloat extends SplType {
}

class SplString extends SplType {
}

class SplEnum extends SplType {
  public function __construct($initial_value, $strict) {}
  public function getConstList($include_default = false) {}
}

class SplBool extends SplEnum {
}
