<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class ZipArchive {

  // Constants
  const CHECKCONS = 4;
  const OVERWRITE = 8;
  const FL_NOCASE = 1;
  const FL_NODIR = 2;
  const FL_COMPRESSED = 4;
  const FL_UNCHANGED = 8;
  const FL_RECOMPRESS = 16;
  const FL_ENCRYPTED = 32;
  const ER_MULTIDISK = 1;
  const ER_RENAME = 2;
  const ER_CLOSE = 3;
  const ER_WRITE = 6;
  const ER_ZIPCLOSED = 8;
  const ER_NOENT = 9;
  const ER_EXISTS = 10;
  const ER_TMPOPEN = 12;
  const ER_MEMORY = 14;
  const ER_CHANGED = 15;
  const ER_COMPNOTSUPP = 16;
  const ER_INVAL = 18;
  const ER_NOZIP = 19;
  const ER_INTERNAL = 20;
  const ER_INCONS = 21;
  const ER_REMOVE = 22;
  const ER_DELETED = 23;
  const ER_ENCRNOTSUPP = 24;
  const ER_RDONLY = 25;
  const ER_NOPASSWD = 26;
  const ER_WRONGPASSWD = 27;
  const CM_DEFAULT = -1;
  const CM_STORE = 0;
  const CM_SHRINK = 1;
  const CM_REDUCE_1 = 2;
  const CM_REDUCE_2 = 3;
  const CM_REDUCE_3 = 4;
  const CM_REDUCE_4 = 5;
  const CM_IMPLODE = 6;
  const CM_DEFLATE = 8;
  const CM_DEFLATE64 = 9;
  const CM_PKWARE_IMPLODE = 10;
  const CM_BZIP2 = 12;
  const CM_TERSE = 18;
  const CM_WAVPACK = 97;
  const CREATE = 1;
  const EXCL = 2;
  const ER_OK = 0;
  const ER_SEEK = 4;
  const ER_READ = 5;
  const ER_CRC = 7;
  const ER_OPEN = 11;
  const ER_ZLIB = 13;
  const ER_EOF = 17;
  const CM_LZMA = 14;
  const CM_LZ77 = 19;
  const CM_PPMD = 98;

  // Properties
  public int $status;
  public int $statusSys;
  public int $numFiles;
  public string $filename;
  public string $comment;

  // Methods
  public function __construct();
  public function addEmptyDir(string $dirname): bool;
  public function addFile(
    string $filename,
    string $localname = "",
    int $start = 0,
    int $length = 0,
  ): bool;
  public function addFromString(string $localname, string $contents): bool;
  public function addGlob(
    string $pattern,
    int $flags = 0,
    array $options = array(),
  ): bool;
  public function addPattern(
    string $pattern,
    string $path = ".",
    array $options = array(),
  ): bool;
  public function close(): bool;
  public function deleteIndex(int $index): bool;
  public function deleteName(string $name): bool;
  public function extractTo(string $destination, $entries = array()): bool;
  public function getArchiveComment(int $flags = 0): string;
  public function getCommentIndex(int $index, int $flags = 0): string;
  public function getCommentName(string $name, int $flags = 0): string;
  public function getFromIndex(
    int $index,
    int $length = 0,
    int $flags = 0,
  ): string;
  public function getFromName(
    string $name,
    int $length = 0,
    int $flags = 0,
  ): string;
  public function getNameIndex(int $index, int $flags = 0): string;
  public function getStatusString(): string;
  public function getStream(string $name): mixed;
  public function locateName(string $name, int $flags = 0): mixed;
  public function open(string $filename, int $flags = 0): mixed;
  public function renameIndex(int $index, string $newname): bool;
  public function renameName(string $name, string $newname): bool;
  public function setArchiveComment(string $comment): bool;
  public function setCommentIndex(int $index, string $comment): bool;
  public function setCommentName(string $name, string $comment): bool;
  public function statIndex(int $index, int $flags = 0): array;
  public function statName(string $name, int $flags = 0): array;
  public function unchangeAll(): bool;
  public function unchangeArchive(): bool;
  public function unchangeIndex(int $index): bool;
  public function unchangeName(string $name): bool;

}

/**
 * Close a ZIP file archive
 */
function zip_close(resource $zip): void;

/**
 * Close a directory entry
 */
function zip_entry_close(resource $zip_entry): bool;

/**
 * Retrieve the compressed size of a directory entry
 */
function zip_entry_compressedsize(resource $zip_entry): int;

/**
 * Retrieve the compression method of a directory entry
 */
function zip_entry_compressionmethod(resource $zip_entry): string;

/**
 * Retrieve the actual file size of a directory entry
 */
function zip_entry_filesize(resource $zip_entry): int;

/**
 * Retrieve the name of a directory entry
 */
function zip_entry_name(resource $zip_entry): string;

/**
 * Open a directory entry for reading
 */
function zip_entry_open(
  resource $zip,
  resource $zip_entry,
  string $mode,
): bool;

/**
 * Read from an open directory entry
 */
function zip_entry_read(
  resource $zip_entry,
  int $length = 1024,
): string;

/**
 * Open a ZIP file archive
 */
function zip_open(string $filename): mixed; // resource or false

/**
 * Read next entry in a ZIP file archive
 */
function zip_read(resource $zip); // resource or false
