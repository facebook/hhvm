<?php

namespace __SystemLib {
  use Phar;

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
    protected Map<string, ArchiveEntryData> $entries = Map { };
    protected ?string $alias;
    protected ?string $stub;
    protected array<string, array<int, int>> $fileOffsets = [];
    protected string $apiVersion = '1.0.0';
    protected $metadata;
    protected ?string $signature;
    protected int $signatureType = Phar::NONE;
    protected $compressed = false;

    // Default implementation, used by Phar- and Tar-based archives
    public function getStream(string $path): resource {
      if (!isset($this->fileOffsets[$path])) {
        throw new PharException("No $path in phar");
      }
      list($offset, $size) = $this->fileOffsets[$path];
      if ($size == 0) {
        return fopen('php://temp', 'w+b');
      }
      $stream = fopen('php://temp', 'w+b');
      //TODO stream slice needed here
      while ($size) {
        $data = $this->stream_get_contents(min(1024, $size), $offset);
        fwrite($stream, $data);
        $size -= strlen($data);
        $offset += strlen($data);
      }
      rewind($stream);
      return $stream;
    }

    abstract public function extractAllTo(string $path);
    abstract public function addFile(string $path, string $archivePath): bool;

    abstract public function __construct(
      string $path,
      bool $preventHaltTokenCheck = true
    );

    public function stat(string $path): ?ArchiveEntryStat {
      return $this->getEntriesMap()->get($path);
    }

    public function getAlias(): ?string {
      return $this->alias;
    }

    public function setAlias(string $alias, int $len) {
      throw new Exception('Not implemented yet');
    }

    public function getStub(): ?string {
      return $this->stub;
    }

    public function setStub(string $stub, int $len = -1) {
      throw new Exception('Not implemented yet');
    }

    public function count(): int {
      return $this->entries->count();
    }

    public function apiVersion(): string {
      return $this->apiVersion;
    }

    public function getMetadata() {
      return $this->metadata;
    }

    public function hasMetadata() {
      return $this->metadata !== null;
    }

    public function getSignature(): ?array {
      switch ($this->signatureType) {
        case Phar::MD5:
          $hash_type = 'MD5';
          break;
        case Phar::SHA1:
          $hash_type = 'SHA-1';
          break;
        case Phar::SHA256:
          $hash_type = 'SHA-256';
          break;
        case Phar::SHA512:
          $hash_type = 'SHA-512';
          break;
        default:
          return null;
      }
      return [
        'hash' => bin2hex($this->signature),
        'hash_type' => $hash_type
      ];
    }

    public function isCompressed() {
      return $this->compressed;
    }

    // Custom methods used by Phar class internally

    public function getEntriesMap(): Map<string, ArchiveEntryStat> {
      return $this->entries;
    }

    // Things used by Phar and Tar to overcome Bzip2 extension's limitation to
    // seek in other way than SEEK_CUR, we need to maintain position manually:(

    private $pos = 0;
    private $stream;
    protected ?string $path;

    protected function open(string $path) {
      $this->path = $path;

      $fp = fopen($path, 'rb');
      $data = fread($fp, 2);
      fclose($fp);

      if ($data === 'BZ') {
        $this->compressed = Phar::BZ2;
        $this->stream = bzopen($path, 'r');
      } else if ($data === "\x1F\x8B") {
        $this->stream = gzopen($path, 'rb');
        $this->compressed = Phar::GZ;
      } else  {
        $this->stream = fopen($path, 'rb');
      }
    }

    protected function stream_get_contents(
      int $maxlength = -1,
      int $offset = -1
    ): ?string {
      if ($offset >= 0) {
        $this->seek($offset);
      }
      $ret = stream_get_contents($this->stream, $maxlength);
      $this->pos += strlen($ret);
      return $ret;
    }

    protected function rewind() {
      $this->seek(0);
    }

    protected function seek(int $position) {
      if ($position < $this->pos && $this->compressed == Phar::BZ2) {
        fclose($this->stream);
        $this->stream = bzopen($this->path, 'r');
        $this->pos = 0;
      }
      fseek($this->stream, $position - $this->pos, SEEK_CUR);
      $this->pos = $position;
    }

    protected function eof(): bool {
      return feof($this->stream);
    }

    public function close(): void {
      if ($this->stream) {
        fclose($this->stream);
        $this->stream = null;
      }
    }
  }
}
