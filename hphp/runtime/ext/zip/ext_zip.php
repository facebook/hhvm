<?hh

/**
 * A file archive, compressed with Zip.
 */
class ZipArchive {

  private ?resource $zipDir = null;
  public string $filename;

  /**
   * Add a new directory
   *
   * @param string $dirname - The directory to add.
   *
   * @return bool -
   */
  <<__Native>>
  public function addEmptyDir(string $dirname): bool;

  /**
   * Adds a file to a ZIP archive from the given path
   *
   * @param string $filename - The path to the file to add.
   * @param string $localname - If supplied, this is the local name
   *   inside the ZIP archive that will override the filename.
   * @param int $start - This parameter is not used but is required to
   *   extend ZipArchive.
   * @param int $length - This parameter is not used but is required to
   *   extend ZipArchive.
   *
   * @return bool -
   */
  <<__Native>>
  public function addFile(string $filename,
                   string $localname = "",
                   int $start = 0,
                   int $length = 0): bool;

  /**
   * Add a file to a ZIP archive using its contents
   *
   * @param string $localname - The name of the entry to create.
   * @param string $contents - The contents to use to create the entry.
   *   It is used in a binary safe mode.
   *
   * @return bool -
   */
  <<__Native>>
  public function addFromString(string $localname,
                         string $contents): bool;

  /**
   * Add files from a directory by glob pattern
   *
   * @param string $pattern -
   * @param int $flags -
   * @param array $options -
   *
   * @return bool -
   */
  <<__Native>>
  public function addGlob(string $pattern,
                   int $flags = 0,
                   darray $options = dict[]): bool;

  /**
   * Add files from a directory by PCRE pattern
   *
   * @param string $pattern -
   * @param string $path -
   * @param array $options -
   *
   * @return bool -
   */
  <<__Native>>
  public function addPattern(string $pattern,
                      string $path = '.',
                      darray $options = dict[]): bool;

  /**
   * Close the active archive (opened or newly created)
   *
   * @return bool -
   */
  <<__Native>>
  public function close(): bool;

  /**
   * delete an entry in the archive using its index
   *
   * @param int $index - Index of the entry to delete.
   *
   * @return bool -
   */
  <<__Native>>
  public function deleteIndex(int $index): bool;

  /**
   * delete an entry in the archive using its name
   *
   * @param string $name - Name of the entry to delete.
   *
   * @return bool -
   */
  <<__Native>>
  public function deleteName(string $name): bool;

  /**
   * Extract the archive contents
   *
   * @param string $destination - Location where to extract the files.
   * @param mixed $entries - The entries to extract. It accepts either a
   *   single entry name or an array of names.
   *
   * @return bool -
   */
  <<__Native>>
  public function extractTo(string $destination,
                     mixed $entries = vec[]): bool;

  /**
   * Returns the Zip archive comment
   *
   * @param int $flags - If flags is set to ZipArchive::FL_UNCHANGED, the
   *   original unchanged comment is returned.
   *
   * @return string - Returns the Zip archive comment.
   */
  <<__Native>>
  public function getArchiveComment(int $flags = 0): mixed;

  /**
   * Returns the comment of an entry using the entry index
   *
   * @param int $index - Index of the entry
   * @param int $flags - If flags is set to ZipArchive::FL_UNCHANGED, the
   *   original unchanged comment is returned.
   *
   * @return string - Returns the comment on success.
   */
  <<__Native>>
  public function getCommentIndex(int $index,
                           int $flags = 0): mixed;

  /**
   * Returns the comment of an entry using the entry name
   *
   * @param string $name - Name of the entry
   * @param int $flags - If flags is set to ZipArchive::FL_UNCHANGED, the
   *   original unchanged comment is returned.
   *
   * @return string - Returns the comment on success.
   */
  <<__Native>>
  public function getCommentName(string $name,
                          int $flags = 0): mixed;

  /**
   * Returns the entry contents using its index
   *
   * @param int $index - Index of the entry
   * @param int $length - The length to be read from the entry. If 0,
   *   then the entire entry is read.
   * @param int $flags - The flags to use to open the archive. the
   *   following values may be ORed to it.    ZipArchive::FL_UNCHANGED
   *   ZipArchive::FL_COMPRESSED
   *
   * @return string - Returns the contents of the entry on success. null on error.
   */
  <<__Native>>
  public function getFromIndex(int $index,
                        int $length = 0,
                        int $flags = 0): ?string;

  /**
   * Returns the entry contents using its name
   *
   * @param string $name - Name of the entry
   * @param int $length - The length to be read from the entry. If 0,
   *   then the entire entry is read.
   * @param int $flags - The flags to use to open the archive. the
   *   following values may be ORed to it.    ZipArchive::FL_UNCHANGED
   *   ZipArchive::FL_COMPRESSED
   *
   * @return string - Returns the contents of the entry on success.
   */
  <<__Native>>
  public function getFromName(string $name,
                       int $length = 0,
                       int $flags = 0): mixed;

  /**
   * Returns the name of an entry using its index
   *
   * @param int $index - Index of the entry.
   * @param int $flags - If flags is set to ZipArchive::FL_UNCHANGED, the
   *   original unchanged name is returned.
   *
   * @return string - Returns the name on success.
   */
  <<__Native>>
  public function getNameIndex(int $index,
                        int $flags = 0): mixed;

  /**
   * Returns the status error message, system and/or zip messages
   *
   * @return string - Returns a string with the status message on
   *   success.
   */
  <<__Native>>
  public function getStatusString(): mixed;

  /**
   * Get a file handler to the entry defined by its name (read only).
   *
   * @param string $name - The name of the entry to use.
   *
   * @return resource - Returns a file pointer (resource) on success.
   */
  <<__Native>>
  public function getStream(string $name): mixed;

  /**
   * Returns the index of the entry in the archive
   *
   * @param string $name - The name of the entry to look up
   * @param int $flags - The flags are specified by ORing the following
   *   values, or 0 for none of them.    ZipArchive::FL_NOCASE
   *   ZipArchive::FL_NODIR
   *
   * @return int - Returns the index of the entry on success.
   */
  <<__Native>>
  public function locateName(string $name,
                      int $flags = 0): mixed;

  /**
   * Open a ZIP file archive
   *
   * @param string $filename - The file name of the ZIP archive to open.
   * @param int $flags - The mode to use to open the archive.
   *   ZipArchive::OVERWRITE     ZipArchive::CREATE     ZipArchive::EXCL
   *    ZipArchive::CHECKCONS
   *
   * @return mixed - Error codes   Returns TRUE on success or the error
   *   code.    ZipArchive::ER_EXISTS   File already exists.
   *   ZipArchive::ER_INCONS   Zip archive inconsistent.
   *   ZipArchive::ER_INVAL   Invalid argument.     ZipArchive::ER_MEMORY
   *   Malloc failure.     ZipArchive::ER_NOENT   No such file.
   *   ZipArchive::ER_NOZIP   Not a zip archive.     ZipArchive::ER_OPEN
   *   Can't open file.     ZipArchive::ER_READ   Read error.
   *   ZipArchive::ER_SEEK   Seek error.
   */
  <<__Native>>
  public function open(string $filename,
                int $flags = 0): mixed;

  /**
   * Renames an entry defined by its index
   *
   * @param int $index - Index of the entry to rename.
   * @param string $newname - New name.
   *
   * @return bool -
   */
  <<__Native>>
  public function renameIndex(int $index,
                       string $newname): bool;

  /**
   * Renames an entry defined by its name
   *
   * @param string $name - Name of the entry to rename.
   * @param string $newname - New name.
   *
   * @return bool -
   */
  <<__Native>>
  public function renameName(string $name,
                      string $newname): bool;

  /**
   * Set the comment of a ZIP archive
   *
   * @param string $comment - The contents of the comment.
   *
   * @return bool -
   */
  <<__Native>>
  public function setArchiveComment(string $comment): bool;

  /**
   * Set the comment of an entry defined by its index
   *
   * @param int $index - Index of the entry.
   * @param string $comment - The contents of the comment.
   *
   * @return bool -
   */
  <<__Native>>
  public function setCommentIndex(int $index,
                           string $comment): bool;

  /**
   * Set the comment of an entry defined by its name
   *
   * @param string $name - Name of the entry.
   * @param string $comment - The contents of the comment.
   *
   * @return bool -
   */
  <<__Native>>
  public function setCommentName(string $name,
                          string $comment): bool;

  /**
   *
   * Set the compression method of an entry defined by its index.
   *
   * @param int $index - Index of the entry.
   * @param int $comp_method - The compression method. Either
   *   ZipArchive::CM_DEFAULT, ZipArchive::CM_STORE or ZipArchive::CM_DEFLATE.
   * @param int $comp_flags - Compression flags. Currently unused.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function setCompressionIndex(int $index, int $comp_method,
                               int $comp_flags = 0): bool;

  /**
   *
   * Set the compression method of an entry defined by its name.
   *
   * @param string $name - Name of the entry.
   * @param int $comp_method - The compression method. Either
   *   ZipArchive::CM_DEFAULT, ZipArchive::CM_STORE or ZipArchive::CM_DEFLATE.
   * @param int $comp_flags - Compression flags. Currently unused.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function setCompressionName(string $name, int $comp_method,
                              int $comp_flags = 0): bool;

  /**
   *
   * Set the encryption method of an entry defined by its index.
   * NOTE: Zip encryption is not secure. Do not use this method unless forced
   * to by external requirments.
   *
   * @param int $index - Index of the entry.
   * @param int $encryption_method - The encryption method. Either
   *   ZipArchive::EM_NONE, ZipArchive::EM_AES_128, ZipArchive::EM_AES_192, or
   *   ZipArchive::EM_AES_256
   * @param string $password - Password to derive a key for the archive from
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function setEncryptionIndex(int $index, int $encryption_method,
                              string $password): bool;

  /**
   *
   * Set the encryption method of an entry defined by its name.
   * NOTE: Zip encryption is not secure. Do not use this method unless forced
   * to by external requirments.
   *
   * @param int $name - Name of the entry.
   * @param int $encryption_method - The encryption method. Either
   *   ZipArchive::EM_NONE, ZipArchive::EM_AES_128, ZipArchive::EM_AES_192, or
   *   ZipArchive::EM_AES_256
   * @param string $password - Password to derive a key for the archive from
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function setEncryptionName(string $name, int $encryption_method,
                             string $password): bool;

  /**
   * Get the details of an entry defined by its index.
   *
   * @param int $index - Index of the entry
   * @param int $flags - ZipArchive::FL_UNCHANGED may be ORed to it to
   *   request information about the original file in the archive, ignoring
   *   any changes made.
   *
   * @return array - Returns an array containing the entry details.
   */
  <<__Native>>
  public function statIndex(int $index,
                     int $flags = 0): mixed;

  /**
   * Get the details of an entry defined by its name.
   *
   * @param string $name - Name of the entry
   * @param int $flags - The flags argument specifies how the name lookup
   *   should be done. Also, ZipArchive::FL_UNCHANGED may be ORed to it to
   *   request information about the original file in the archive, ignoring
   *   any changes made.    ZipArchive::FL_NOCASE     ZipArchive::FL_NODIR
   *      ZipArchive::FL_UNCHANGED
   *
   * @return array - Returns an array containing the entry details .
   */
  <<__Native>>
  public function statName(string $name,
                    int $flags = 0): mixed;

  /**
   * Undo all changes done in the archive
   *
   * @return bool -
   */
  <<__Native>>
  public function unchangeAll(): bool;

  /**
   * Revert all global changes done in the archive.
   *
   * @return bool -
   */
  <<__Native>>
  public function unchangeArchive(): bool;

  /**
   * Revert all changes done to an entry at the given index
   *
   * @param int $index - Index of the entry.
   *
   * @return bool -
   */
  <<__Native>>
  public function unchangeIndex(int $index): bool;

  /**
   * Revert all changes done to an entry with the given name.
   *
   * @param string $name - Name of the entry.
   *
   * @return bool -
   */
  <<__Native>>
  public function unchangeName(string $name): bool;

}

/**
 * Close a ZIP file archive
 *
 * @param resource $zip - A ZIP file previously opened with zip_open().
 *
 * @return void -
 */
<<__Native>>
function zip_close(resource $zip): mixed;

/**
 * Close a directory entry
 *
 * @param resource $zip_entry - A directory entry previously opened
 *   zip_entry_open().
 *
 * @return bool -
 */
<<__Native>>
function zip_entry_close(resource $zip_entry): bool;

/**
 * Retrieve the compressed size of a directory entry
 *
 * @param resource $zip_entry - A directory entry returned by zip_read().
 *
 * @return int - The compressed size.
 */
<<__Native>>
function zip_entry_compressedsize(resource $zip_entry): mixed;

/**
 * Retrieve the compression method of a directory entry
 *
 * @param resource $zip_entry - A directory entry returned by zip_read().
 *
 * @return string - The compression method.
 */
<<__Native>>
function zip_entry_compressionmethod(resource $zip_entry): mixed;

/**
 * Retrieve the actual file size of a directory entry
 *
 * @param resource $zip_entry - A directory entry returned by zip_read().
 *
 * @return int - The size of the directory entry.
 */
<<__Native>>
function zip_entry_filesize(resource $zip_entry): mixed;

/**
 * Retrieve the name of a directory entry
 *
 * @param resource $zip_entry - A directory entry returned by zip_read().
 *
 * @return string - The name of the directory entry.
 */
<<__Native>>
function zip_entry_name(resource $zip_entry): mixed;

/**
 * Open a directory entry for reading
 *
 * @param resource $zip - A valid resource handle returned by zip_open().
 * @param resource $zip_entry - A directory entry returned by zip_read().
 * @param string $mode - Any of the modes specified in the documentation
 *   of fopen().    Currently, mode is ignored and is always "rb". This is
 *   due to the fact that zip support in PHP is read only access.
 *
 * @return bool - Unlike fopen() and other similar functions, the return
 *   value of zip_entry_open() only indicates the result of the operation
 *   and is not needed for reading or closing the directory entry.
 */
<<__Native>>
function zip_entry_open(resource $zip,
                        resource $zip_entry,
                        string $mode): bool;

/**
 * Read from an open directory entry
 *
 * @param resource $zip_entry - A directory entry returned by zip_read().
 * @param int $length - The number of bytes to return.    This should be
 *   the uncompressed length you wish to read.
 *
 * @return string - Returns the data read, empty string on end of a file,
 *   or FALSE on error.
 */
<<__Native>>
function zip_entry_read(resource $zip_entry,
                        int $length = 1024): mixed;

/**
 * Open a ZIP file archive
 *
 * @param string $filename - The file name of the ZIP archive to open.
 *
 * @return resource - Returns a resource handle for later use with
 *   zip_read() and zip_close() or returns the number of error if filename
 *   does not exist or in case of other error.
 */
<<__Native>>
function zip_open(string $filename): mixed;

/**
 * Read next entry in a ZIP file archive
 *
 * @param resource $zip - A ZIP file previously opened with zip_open().
 *
 * @return resource - Returns a directory entry resource for later use
 *   with the zip_entry_... functions, or FALSE if there are no more
 *   entries to read, or an error code if an error occurred.
 */
<<__Native>>
function zip_read(resource $zip): mixed;
