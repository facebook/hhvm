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
function spl_classes(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function spl_object_hash(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_object_pointer(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_get_this(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function class_implements(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  bool $autoload = true,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function class_parents(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  bool $autoload = true,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function class_uses(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  bool $autoload = true,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function iterator_apply(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  HH\FIXME\MISSING_PARAM_TYPE $func,
  HH\FIXME\MISSING_PARAM_TYPE $params = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function iterator_count(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function iterator_to_array(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  bool $use_keys = true,
): HH\FIXME\MISSING_RETURN_TYPE;

class SplDoublyLinkedList<T>
  implements Iterator<T>, ArrayAccess<int, T>, Countable {
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

  public function offsetExists(/*int*/ HH\FIXME\MISSING_PARAM_TYPE $key): bool;
  public function offsetGet(/*int*/ HH\FIXME\MISSING_PARAM_TYPE $key): T;
  public function offsetSet(/*int*/
    HH\FIXME\MISSING_PARAM_TYPE $key,
    T $val,
  ): void;
  public function offsetUnset(/*int*/ HH\FIXME\MISSING_PARAM_TYPE $key): void;

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

class SplFileObject
  extends SplFileInfo
  implements
    SeekableIterator<~string>,
    RecursiveIterator<~string> {

  // Constants
  const int DROP_NEW_LINE;
  const int READ_AHEAD;
  const int SKIP_EMPTY;
  const int READ_CSV;

  // Methods
  public function __construct(
    string $filename,
    string $open_mode = "r",
    bool $use_include_path = false,
    ?resource $context = null,
  );
  /** @return string|array violates Iterator interface */
  public function current(): ~string;
  public function eof(): bool;
  public function fflush(): bool;
  public function fgetc(): string;
  public function fgetcsv(
    string $delimiter = ",",
    string $enclosure = "\"",
    string $escape = "\\",
  ): varray<mixed>;
  public function fgets(): HH\FIXME\MISSING_RETURN_TYPE;
  public function fgetss(?string $allowable_tags = null): string;
  public function flock(int $operation, inout mixed $wouldblock): bool;
  public function fpassthru(): int;
  public function fputcsv(
    varray<mixed> $fields,
    string $delimiter = ",",
    string $enclosure = '"',
  ): int;
  public function fread(int $length): string;
  public function fscanf(string $format): varray<mixed>;
  public function fseek(int $offset, int $whence = SEEK_SET): int;
  public function fstat(): darray<arraykey, mixed>;
  public function ftell(): int;
  public function ftruncate(int $size): bool;
  public function fwrite(string $str, int $length): int;
  public function getCsvControl(): varray<mixed>;
  public function getFlags(): int;
  public function getMaxLineLen(): int;
  public function setCsvControl(
    string $delimiter = ",",
    string $enclosure = "\"",
    string $escape = "\\",
  ): void;
  /* (always) returns null */
  public function getChildren(): null;
  public function hasChildren(): bool;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function seek(int $line_pos): void;
  public function setFlags(int $flags): void;
  public function setMaxLineLen(int $max_len): void;
  public function valid(): bool;
}

class SplTempFileObject extends SplFileObject {

  // Methods
  public function __construct(?int $max_memory = null);
}

class DirectoryIterator
  extends SplFileInfo
  implements SeekableIterator<~SplFileInfo> {

  // Methods
  public function __construct(string $path);
  public function current(): ~SplFileInfo;
  public function isDot(): bool;
  public function next(): void;
  public function rewind(): void;
  public function seek(int $position): void;
  public function valid(): bool;
  public function key(): mixed;
}

class NoRewindIterator<T> extends IteratorIterator<T> {
}

class InfiniteIterator<T> extends IteratorIterator<T> {
}

class GlobIterator extends FilesystemIterator implements Countable {
  public function count(): int {}
}

class SplStack<T> extends SplDoublyLinkedList<T> {
}

class SplPriorityQueue<Tv, Tp>
  implements Iterator<~Tv>, Countable {
  const int EXTR_BOTH;
  const int EXTR_PRIORITY;
  const int EXTR_DATA;
  public function compare(Tp $a, Tp $b): int {}
  public function count(): int {}
  public function current(): ~Tv {}
  public function extract(): ~Tv {}
  public function insert(Tv $value, Tp $priority): bool {}
  public function isEmpty(): bool {}
  public function key(): int {}
  public function next(): void {}
  public function recoverFromCorruption(): bool {}
  public function rewind(): void {}
  public function setExtractFlags(int $flags): int {}
  public function top(): ~Tv {}
  public function valid(): bool {}
}

interface SplObserver {
  public function update(
    SplSubject $SplSubject,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
}

interface SplSubject {
  public function attach(
    SplObserver $SplObserver,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function detach(
    SplObserver $SplObserver,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function notify(): HH\FIXME\MISSING_RETURN_TYPE {}
}
