<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function spl_classes();
function spl_object_hash($obj);
function hphp_object_pointer($obj);
function hphp_get_this();
function class_implements($obj, $autoload = true);
function class_parents($obj, $autoload = true);
function class_uses($obj, $autoload = true);
function iterator_apply($obj, $func, $params = null);
function iterator_count($obj);
function iterator_to_array($obj, $use_keys = true);
function spl_autoload_call($class_name);
function spl_autoload_extensions($file_extensions = null);
function spl_autoload_functions();
function spl_autoload_register($autoload_function = null, $throws = true, $prepend = false);
function spl_autoload_unregister($autoload_function);
function spl_autoload($class_name, $file_extensions = null);

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
  /* returns string|array ... violates Iterator interface */
  public function current();
  public function eof(): bool;
  public function fflush(): bool;
  public function fgetc(): string;
  public function fgetcsv(
    string $delimiter = ",",
    string $enclosure = "\"",
    string $escape = "\\",
  ): array;
  public function fgets(): string;
  public function fgetss(?string $allowable_tags = null): string;
  public function flock(int $operation, mixed &$wouldblock = false): bool;
  public function fpassthru(): int;
  public function fputcsv(
    array $fields,
    string $delimiter = ",",
    string $enclosure = '"',
  ): int;
  public function fread(int $length): string;
  public function fscanf(string $format, ... ): mixed;
  public function fseek(int $offset, int $whence = SEEK_SET): int;
  public function fstat(): array;
  public function ftell(): int;
  public function ftruncate(int $size): bool;
  public function fwrite(string $str, int $length): int;
  public function getCsvControl(): array;
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
