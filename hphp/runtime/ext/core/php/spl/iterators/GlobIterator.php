<?hh

class GlobIterator extends FilesystemIterator
  implements SeekableIterator, Countable {

  public function __construct($path, $flags = null) {
    if ($flags is null) {
      $flags = FilesystemIterator::KEY_AS_PATHNAME |
        FilesystemIterator::CURRENT_AS_FILEINFO;
    }
    // prepend the "glob://" prefix if it isn't there
    $prefix = 'glob://';
    $prefix_len = strlen($prefix);

    if (strncmp($path, $prefix, $prefix_len) !== 0) {
      $path = $prefix . $path;
    }

    parent::__construct($path, $flags);
  }

  <<__Native>>
  public function count(): int;
}
