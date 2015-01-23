<?php

namespace __SystemLib {
  class ArchiveEntryStat {
    public function __construct(
      public ?int $crc,
      public int $size,
      public ?int $compresedSize,
      public int $timestamp,
    ) {
    }
  }

  abstract class ArchiveHandler {
    abstract public function getContentsList(): Map<string, ArchiveEntryStat>;
    abstract public function read(string $path): ?string;
    abstract public function extractAllTo(string $path);
    abstract public function addFile(string $path, string $archivePath): bool;
    abstract public function close(): void;

    public function stat($path): ?ArchiveEntryStat {
      $contents = $this->getContentsList();
      if ($contents->contains($path)) {
        return $contents[$path];
      }
      return null;
    }
  }
}
