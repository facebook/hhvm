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

class RecursiveIteratorIterator<Tk, Tv> implements OuterIterator<Tv>, KeyedIterator<Tk, Tv> {
  const int LEAVES_ONLY = 0;
  const int SELF_FIRST = 1;
  const int CHILD_FIRST = 2;
  const int CATCH_GET_CHILD = 16;
  public function __construct(Traversable<Tv> $iterator, $mode = null, $flags = null) {}
  public function beginChildren() {}
  public function beginIteration() {}
  public function callGetChildren() {}
  public function callHasChildren() {}
  public function current() {}
  public function endChildren() {}
  public function endIteration() {}
  public function getDepth() {}
  public function getInnerIterator() {}
  public function getMaxDepth() {}
  public function getSubIterator($level = null) {}
  public function key() {}
  public function next() {}
  public function nextElement() {}
  public function rewind() {}
  public function setMaxDepth($max_depth = null) {}
  public function valid() {}
}

class IteratorIterator<T> implements OuterIterator<T> {
  public function __construct(Traversable $iterator) {}
  public function current() {}
  public function getInnerIterator() {}
  public function key() {}
  public function next() {}
  public function rewind() {}
  public function valid() {}
}

abstract class FilterIterator<T> extends IteratorIterator<T> {
  abstract public function accept();
}

abstract class RecursiveFilterIterator<T> extends FilterIterator<T> implements RecursiveIterator<T> {
  public function getChildren() {}
  public function hasChildren() {}
}

class CallbackFilterIterator<T> extends FilterIterator<T> {
  public function accept() {}
}

class RecursiveCallbackFilterIterator<T> extends CallbackFilterIterator<T> implements RecursiveIterator<T> {
  public function getChildren() {}
  public function hasChildren() {}
}

class ParentIterator<T> extends RecursiveFilterIterator<T> {
  public function accept() {}
}

class LimitIterator<T> extends IteratorIterator<T> {
  public function getPosition() {}
  public function seek($position) {}
}

class CachingIterator<Tk, Tv> extends IteratorIterator<Tv> implements ArrayAccess<Tk, Tv>, Countable {
  const int CALL_TOSTRING = 1;
  const int CATCH_GET_CHILD = 16;
  const int TOSTRING_USE_KEY = 2;
  const int TOSTRING_USE_CURRENT = 4;
  const int TOSTRING_USE_INNER = 8;
  const int FULL_CACHE = 256;
  public function __toString() {}
  public function count() {}
  public function getCache() {}
  public function getFlags() {}
  public function hasNext() {}
  public function offsetExists($index) {}
  public function offsetGet($index) {}
  public function offsetSet($index, $newval) {}
  public function offsetUnset($index) {}
  public function setFlags($flags) {}
}

class RecursiveCachingIterator<Tk, Tv> extends CachingIterator<Tk, Tv> implements RecursiveIterator<Tv> {
  public function getChildren() {}
  public function hasChildren() {}
}

class NoRewindIterator<T> extends IteratorIterator<T> {
}

class AppendIterator<T> extends IteratorIterator<T> {
  public function append(Iterator $iterator) {}
  public function getArrayIterator() {}
  public function getIteratorIndex() {}
}

class InfiniteIterator<T> extends IteratorIterator<T> {
}

class RegexIterator<T> extends FilterIterator<T> {
  const int USE_KEY = 1;
  const int INVERT_MATCH = 2;
  const int MATCH = 0;
  const int GET_MATCH = 1;
  const int ALL_MATCHES = 2;
  const int SPLIT = 3;
  const int REPLACE = 4;
  public $replacement;
  public function accept() {}
  public function getFlags() {}
  public function getMode() {}
  public function getPregFlags() {}
  public function getRegex() {}
  public function setFlags($flags) {}
  public function setMode($mode) {}
  public function setPregFlags($preg_flags) {}
}

class RecursiveRegexIterator<T> extends RegexIterator<T> implements RecursiveIterator<T> {
  public function getChildren() {}
  public function hasChildren() {}
}

class EmptyIterator<T> implements Iterator<T> {
  public function current() {}
  public function key() {}
  public function next() {}
  public function rewind() {}
  public function valid() {}
}

class RecursiveTreeIterator<Tk, Tv> extends RecursiveIteratorIterator<Tk, Tv> {
  const int BYPASS_CURRENT = 4;
  const int BYPASS_KEY = 8;
  const int PREFIX_LEFT = 0;
  const int PREFIX_MID_HAS_NEXT = 1;
  const int PREFIX_MID_LAST = 2;
  const int PREFIX_END_HAS_NEXT = 3;
  const int PREFIX_END_LAST = 4;
  const int PREFIX_RIGHT = 5;
  public function getEntry() {}
  public function getPostfix() {}
  public function getPrefix() {}
  public function setPostfix() {}
  public function setPrefixPart($part, $value) {}
}

class ArrayObject<Tk, Tv> implements IteratorAggregate<Tv>, ArrayAccess<Tk, Tv>, Serializable, Countable {
  const int STD_PROP_LIST = 1;
  const int ARRAY_AS_PROPS = 2;
  public function __construct($array) {}
  public function append($value) {}
  public function asort() {}
  public function count() {}
  public function exchangeArray($array) {}
  public function getArrayCopy() {}
  public function getFlags() {}
  public function getIterator() {}
  public function getIteratorClass() {}
  public function ksort() {}
  public function natcasesort() {}
  public function natsort() {}
  public function offsetExists($index) {}
  public function offsetGet($index) {}
  public function offsetSet($index, $newval) {}
  public function offsetUnset($index) {}
  public function serialize() {}
  public function setFlags($flags) {}
  public function setIteratorClass($iteratorClass) {}
  public function uasort($cmp_function) {}
  public function uksort($cmp_function) {}
  public function unserialize($serialized) {}
}

class RecursiveArrayIterator<T> extends ArrayIterator<T> implements RecursiveIterator<T> {
  const int CHILD_ARRAYS_ONLY = 4;
  public function getChildren() {}
  public function hasChildren() {}
}

class FilesystemIterator extends DirectoryIterator {
  const int CURRENT_MODE_MASK = 240;
  const int CURRENT_AS_PATHNAME = 32;
  const int CURRENT_AS_FILEINFO = 0;
  const int CURRENT_AS_SELF = 16;
  const int KEY_MODE_MASK = 3840;
  const int KEY_AS_PATHNAME = 0;
  const int FOLLOW_SYMLINKS = 512;
  const int KEY_AS_FILENAME = 256;
  const int NEW_CURRENT_AND_KEY = 256;
  const int OTHER_MODE_MASK = 12288;
  const int SKIP_DOTS = 4096;
  const int UNIX_PATHS = 8192;
  public function __construct($path, $flags) {}
  public function getFlags() {}
  public function setFlags($flags = null) {}
}

class RecursiveDirectoryIterator<T> extends FilesystemIterator implements RecursiveIterator<DirectoryIterator> {
  public function __construct($path, $flags = null) {}
  public function getChildren() {}
  public function getSubPath() {}
  public function getSubPathname() {}
  public function hasChildren($allow_links = null) {}
}

class GlobIterator extends FilesystemIterator implements Countable {
  public function count() {}
}

class SplStack<T> extends SplDoublyLinkedList<T> {
}

abstract class SplHeap<T> implements Iterator<T>, Countable {
  abstract public function compare($a, $b);
  public function count() {}
  public function current() {}
  public function extract() {}
  public function insert($value) {}
  public function isEmpty() {}
  public function key() {}
  public function next() {}
  public function recoverFromCorruption() {}
  public function rewind() {}
  public function top() {}
  public function valid() {}
}

class SplMinHeap<T> extends SplHeap<T> {
  public function compare($a, $b) {}
}

class SplMaxHeap<T> extends SplHeap<T> {
  public function compare($a, $b) {}
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

class SplFixedArray<Tk, Tv> implements Iterator<Tv>, ArrayAccess<Tk, Tv>, Countable {
  public function __construct($size = null) {}
  public function __wakeup() {}
  public function count() {}
  public function current() {}
  public static function fromArray($data, $save_indexes = null) {}
  public function getSize() {}
  public function key() {}
  public function next() {}
  public function offsetExists($index) {}
  public function offsetGet($index) {}
  public function offsetSet($index, $newval) {}
  public function offsetUnset($index) {}
  public function rewind() {}
  public function setSize($value) {}
  public function toArray() {}
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

class SplObjectStorage<Tk, Tv> implements Countable, Iterator<Tv>, Serializable, ArrayAccess<Tk, Tv> {
  public function addAll($object) {}
  public function attach($object, $inf = null) {}
  public function contains($object) {}
  public function count() {}
  public function current() {}
  public function detach($object) {}
  public function getHash($object) {}
  public function getInfo() {}
  public function key() {}
  public function next() {}
  public function offsetExists($object) {}
  public function offsetGet($object) {}
  public function offsetSet($object, $inf = null) {}
  public function offsetUnset($object) {}
  public function removeAll($object) {}
  public function removeAllExcept($object) {}
  public function rewind() {}
  public function serialize() {}
  public function setInfo($info) {}
  public function unserialize($serialized) {}
  public function valid() {}
}

class MultipleIterator<T> implements Iterator<T> {
  const int MIT_NEED_ANY = 0;
  const int MIT_NEED_ALL = 1;
  const int MIT_KEYS_NUMERIC = 0;
  const int MIT_KEYS_ASSOC = 2;
  public function __construct($flags) {}
  public function attachIterator(Iterator $iterator, $infos = null) {}
  public function containsIterator(Iterator $iterator) {}
  public function countIterators() {}
  public function current() {}
  public function detachIterator(Iterator $iterator) {}
  public function getFlags() {}
  public function key() {}
  public function next() {}
  public function rewind() {}
  public function setFlags($flags) {}
  public function valid() {}
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
