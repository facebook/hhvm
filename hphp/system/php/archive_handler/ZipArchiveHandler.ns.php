<?php

namespace __SystemLib {
  use Phar;
  use PharException;
  use ZipArchive;

  final class ZipArchiveHandler extends ArchiveHandler {
    private ZipArchive $za;

    public function __construct(
      string $path,
      bool $preventHaltTokenCheck = true
    ) {
      $this->za = new ZipArchive();
      if (file_exists($path)) {
        $this->za->open($path);
        $this->fillEntries();
        if (
          !$preventHaltTokenCheck &&
          strpos($this->stub, Phar::HALT_TOKEN) === false
        ) {
          throw new PharException(
            Phar::HALT_TOKEN.' must be declared in a phar'
          );
        }
      } else {
        $this->za->open($path, ZipArchive::CREATE);
      }
    }

    private function fillEntries() {
      for ($i = 0; $i < $this->za->numFiles; ++$i) {
        $fname = $this->za->getNameIndex($i);
        if (substr($fname, -1) === '/') {
          continue;
        }
        // Hidden .phar directory should not appear in files listing
        if (strpos($fname, '.phar') === 0) {
          if ($fname == '.phar/stub.php') {
            $this->stub = $this->za->getFromName($fname);
          } else if ($fname == '.phar/alias.txt') {
            $this->alias = $this->za->getFromName($fname);
          }
          continue;
        }
        $stat = $this->za->statIndex($i);
        $this->entries[$fname] = new ArchiveEntryStat(
          $stat['crc'],
          $stat['size'],
          $stat['comp_size'] ?: $stat['size'],
          $stat['mtime'],
        );
      }
    }

    public function close(): void {
      $this->za->close();
    }

    public function getStream(string $path): resource {
      $stream = $this->za->getStream($path);
      if (!is_resource($stream)) {
        throw new PharException("No $path in phar");
      }
      return $stream;
    }

    public function extractAllTo(string $path) {
      return $this->za->extractTo($path);
    }

    public function addFile(string $path, string $archive_path): bool {
      return $this->za->addFile($path, $archive_path);
    }
  }
}
