<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class ZipArchive {

  // Constants
  const int CHECKCONS = 4;
  const int OVERWRITE = 8;
  const int FL_NOCASE = 1;
  const int FL_NODIR = 2;
  const int FL_COMPRESSED = 4;
  const int FL_UNCHANGED = 8;
  const int FL_RECOMPRESS = 16;
  const int FL_ENCRYPTED = 32;
  const int ER_MULTIDISK = 1;
  const int ER_RENAME = 2;
  const int ER_CLOSE = 3;
  const int ER_WRITE = 6;
  const int ER_ZIPCLOSED = 8;
  const int ER_NOENT = 9;
  const int ER_EXISTS = 10;
  const int ER_TMPOPEN = 12;
  const int ER_MEMORY = 14;
  const int ER_CHANGED = 15;
  const int ER_COMPNOTSUPP = 16;
  const int ER_INVAL = 18;
  const int ER_NOZIP = 19;
  const int ER_INTERNAL = 20;
  const int ER_INCONS = 21;
  const int ER_REMOVE = 22;
  const int ER_DELETED = 23;
  const int ER_ENCRNOTSUPP = 24;
  const int ER_RDONLY = 25;
  const int ER_NOPASSWD = 26;
  const int ER_WRONGPASSWD = 27;
  const int CM_DEFAULT = -1;
  const int CM_STORE = 0;
  const int CM_SHRINK = 1;
  const int CM_REDUCE_1 = 2;
  const int CM_REDUCE_2 = 3;
  const int CM_REDUCE_3 = 4;
  const int CM_REDUCE_4 = 5;
  const int CM_IMPLODE = 6;
  const int CM_DEFLATE = 8;
  const int CM_DEFLATE64 = 9;
  const int CM_PKWARE_IMPLODE = 10;
  const int CM_BZIP2 = 12;
  const int CM_TERSE = 18;
  const int CM_WAVPACK = 97;
  const int CREATE = 1;
  const int EXCL = 2;
  const int ER_OK = 0;
  const int ER_SEEK = 4;
  const int ER_READ = 5;
  const int ER_CRC = 7;
  const int ER_OPEN = 11;
  const int ER_ZLIB = 13;
  const int ER_EOF = 17;
  const int CM_LZMA = 14;
  const int CM_LZ77 = 19;
  const int CM_PPMD = 98;
  const int EM_NONE = 0;
  const int EM_AES_128 = 257;
  const int EM_AES_192 = 258;
  const int EM_AES_256 = 259;

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
    darray $options = darray[],
  ): bool;
  public function addPattern(
    string $pattern,
    string $path = ".",
    darray $options = darray[],
  ): bool;
  public function close(): bool;
  public function deleteIndex(int $index): bool;
  public function deleteName(string $name): bool;
  public function extractTo(string $destination, $entries = varray[]): bool;
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
  public function setCompressionIndex(
    int $index,
    int $comp_method,
    int $comp_flags = 0,
  ): bool;
  public function setEncryptionIndex(
    int $index,
    int $encryption_method,
    string $password,
  ): bool;
  public function setEncryptionName(
    string $name,
    int $encryption_method,
    string $password,
  ): bool;
  public function statIndex(int $index, int $flags = 0): darray;
  public function statName(string $name, int $flags = 0): darray;
  public function unchangeAll(): bool;
  public function unchangeArchive(): bool;
  public function unchangeIndex(int $index): bool;
  public function unchangeName(string $name): bool;

}

/**
 * Close a ZIP file archive
 */
<<__PHPStdLib>>
function zip_close(resource $zip): void;

/**
 * Close a directory entry
 */
<<__PHPStdLib>>
function zip_entry_close(resource $zip_entry): bool;

/**
 * Retrieve the compressed size of a directory entry
 */
<<__PHPStdLib>>
function zip_entry_compressedsize(resource $zip_entry): int;

/**
 * Retrieve the compression method of a directory entry
 */
<<__PHPStdLib>>
function zip_entry_compressionmethod(resource $zip_entry): string;

/**
 * Retrieve the actual file size of a directory entry
 */
<<__PHPStdLib>>
function zip_entry_filesize(resource $zip_entry): int;

/**
 * Retrieve the name of a directory entry
 */
<<__PHPStdLib>>
function zip_entry_name(resource $zip_entry): string;

/**
 * Open a directory entry for reading
 */
<<__PHPStdLib>>
function zip_entry_open(
  resource $zip,
  resource $zip_entry,
  string $mode,
): bool;

/**
 * Read from an open directory entry
 */
<<__PHPStdLib>>
function zip_entry_read(
  resource $zip_entry,
  int $length = 1024,
): string;

/**
 * Open a ZIP file archive
 */
<<__PHPStdLib>>
function zip_open(string $filename): mixed; // resource or false

/**
 * Read next entry in a ZIP file archive
 */
<<__PHPStdLib>>
function zip_read(resource $zip); // resource or false
