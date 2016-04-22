<?php

namespace __SystemLib {
  use PharException;
  use ZipArchive;

  final class ZipArchiveHandler extends ArchiveHandler {
    private ZipArchive $za;

    public function __construct(string $path) {
      $this->za = new ZipArchive();
      if (file_exists($path)) {
        $this->za->open($path);
        $this->fillEntries();
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

    public function read(string $path): ?string {
      $data = $this->za->getFromName($path);
      if ($data === false) {
        return null;
      }
      return $data;
    }

    public function extractAllTo(string $path) {
      return $this->za->extractTo($path);
    }

    public function addFile(string $path, string $archive_path): bool {
      return $this->za->addFile($path, $archive_path);
    }

    // Custom methods used by Phar class internally

    public function getFileContents(string $filename): string {
      if ($this->za->locateName($filename) === false) {
        throw new PharException("No $filename in phar");
      }
      return $this->getFromName($filename);
    }
  }
}
