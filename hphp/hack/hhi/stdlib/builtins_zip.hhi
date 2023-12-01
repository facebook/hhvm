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
  const int CHECKCONS;
  const int OVERWRITE;
  const int FL_NOCASE;
  const int FL_NODIR;
  const int FL_COMPRESSED;
  const int FL_UNCHANGED;
  const int FL_RECOMPRESS;
  const int FL_ENCRYPTED;
  const int ER_MULTIDISK;
  const int ER_RENAME;
  const int ER_CLOSE;
  const int ER_WRITE;
  const int ER_ZIPCLOSED;
  const int ER_NOENT;
  const int ER_EXISTS;
  const int ER_TMPOPEN;
  const int ER_MEMORY;
  const int ER_CHANGED;
  const int ER_COMPNOTSUPP;
  const int ER_INVAL;
  const int ER_NOZIP;
  const int ER_INTERNAL;
  const int ER_INCONS;
  const int ER_REMOVE;
  const int ER_DELETED;
  const int ER_ENCRNOTSUPP;
  const int ER_RDONLY;
  const int ER_NOPASSWD;
  const int ER_WRONGPASSWD;
  const int CM_DEFAULT;
  const int CM_STORE;
  const int CM_SHRINK;
  const int CM_REDUCE_1;
  const int CM_REDUCE_2;
  const int CM_REDUCE_3;
  const int CM_REDUCE_4;
  const int CM_IMPLODE;
  const int CM_DEFLATE;
  const int CM_DEFLATE64;
  const int CM_PKWARE_IMPLODE;
  const int CM_BZIP2;
  const int CM_TERSE;
  const int CM_WAVPACK;
  const int CREATE;
  const int EXCL;
  const int ER_OK;
  const int ER_SEEK;
  const int ER_READ;
  const int ER_CRC;
  const int ER_OPEN;
  const int ER_ZLIB;
  const int ER_EOF;
  const int CM_LZMA;
  const int CM_LZ77;
  const int CM_PPMD;
  const int EM_NONE;
  const int EM_AES_128;
  const int EM_AES_192;
  const int EM_AES_256;

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
    darray<arraykey, mixed> $options = dict[],
  ): bool;
  public function addPattern(
    string $pattern,
    string $path = ".",
    darray<arraykey, mixed> $options = dict[],
  ): bool;
  public function close(): bool;
  public function deleteIndex(int $index): bool;
  public function deleteName(string $name): bool;
  public function extractTo(
    string $destination,
    HH\FIXME\MISSING_PARAM_TYPE $entries = vec[],
  ): bool;
  public function getArchiveComment(int $flags = 0): string;
  public function getCommentIndex(int $index, int $flags = 0): string;
  public function getCommentName(string $name, int $flags = 0): string;
  public function getFromIndex(
    int $index,
    int $length = 0,
    int $flags = 0,
  ): ?string;
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
  public function statIndex(int $index, int $flags = 0): darray<arraykey, mixed>;
  public function statName(string $name, int $flags = 0): darray<arraykey, mixed>;
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
function zip_entry_open(resource $zip, resource $zip_entry, string $mode): bool;

/**
 * Read from an open directory entry
 */
<<__PHPStdLib>>
function zip_entry_read(resource $zip_entry, int $length = 1024): string;

/**
 * Open a ZIP file archive
 */
<<__PHPStdLib>>
function zip_open(string $filename): mixed; // resource or false

/**
 * Read next entry in a ZIP file archive
 */
<<__PHPStdLib>>
function zip_read(
  resource $zip,
): HH\FIXME\MISSING_RETURN_TYPE; // resource or false
