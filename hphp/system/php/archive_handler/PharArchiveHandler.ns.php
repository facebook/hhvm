<?php

namespace __SystemLib {
  use Phar;
  use PharException;

  final class PharArchiveHandler extends ArchiveHandler {
    private array<string, array> $fileInfo = array();
    private $archiveFlags;
    public function __construct(
      string $path,
      bool $preventHaltTokenCheck = true
    ) {
      $this->open($path);
      $pos = $this->haltTokenPosition();
      if ($pos) {
        $this->stub = $this->stream_get_contents($pos, 0);
        $pos += strlen(Phar::HALT_TOKEN);
        // *sigh*. We have to allow whitespace then ending the file
        // before we start the manifest
        while ($this->stream_get_contents(1, $pos) == ' ') {
          $pos += 1;
        }
        if (
          $this->stream_get_contents(1, $pos) == '?' &&
          $this->stream_get_contents(1, $pos + 1) == '>'
        ) {
          $pos += 2;
        }
        while ($this->stream_get_contents(1, $pos) == "\r") {
          $pos += 1;
        }
        while ($this->stream_get_contents(1, $pos) == "\n") {
          $pos += 1;
        }
      } else if (!$preventHaltTokenCheck) {
        throw new PharException(Phar::HALT_TOKEN.' must be declared in a phar');
      }

      $this->parsePhar($pos);
    }

    private function haltTokenPosition(): int {
      $offset = 0;
      $prev_data = '';
      $next_data = '';
      do {
        $offset += strlen($prev_data);
        $prev_data = $next_data;
        $next_data = $this->stream_get_contents(1024);
        $pos = strpos($prev_data.$next_data, Phar::HALT_TOKEN);
        if ($pos !== false) {
          return $offset + $pos;
        }
      } while (!$this->eof());
      // We'll not get 0 as position of real token because of `<?php` at start
      return 0;
    }

    private function parsePhar(&$pos) {
      $start = $pos;
      $len = $this->bytesToInt($pos, 4);
      $count = $this->bytesToInt($pos, 4);
      $this->apiVersion = $this->bytesToInt($pos, 2);
      $this->archiveFlags = $this->bytesToInt($pos, 4);
      $alias_len = $this->bytesToInt($pos, 4);
      $this->alias = $this->substr($pos, $alias_len);
      $metadata_len = $this->bytesToInt($pos, 4);
      if ($metadata_len > 0) {
        $this->metadata = unserialize(
          $this->substr($pos, $metadata_len)
        );
      }
      $this->parseFileInfo($count, $pos);
      if ($pos != $start + $len + 4) {
        throw new PharException(
          "Malformed manifest. Expected $len bytes, got $pos"
        );
      }
      foreach ($this->fileInfo as $filename => $info) {
        $this->fileOffsets[$filename] = [$pos, $info[2]];
        $pos += $info[2];
        $this->entries[$filename] = new ArchiveEntryStat(
          $info[3],
          $info[0],
          $info[2],
          $info[1]
        );
      }

      $signatureStart = $pos;
      $signature = $this->stream_get_contents(-1, $pos);
      $signatureSize = strlen($signature);

      // Try to see if there is a signature
      if ($this->archiveFlags & Phar::SIGNATURE) {
        if (substr($signature, -4) !== 'GBMB') {
          // Not even the GBMB and the flags?
          throw new PharException('phar has a broken signature');
        }

        $pos = $signatureStart + $signatureSize - 8;
        $this->signatureType = $this->bytesToInt($pos, 4);
        switch ($this->signatureType) {
          case Phar::MD5:
            $digestSize = 16;
            $digestName = 'md5';
            break;
          case Phar::SHA1:
            $digestSize = 20;
            $digestName = 'sha1';
            break;
          case Phar::SHA256:
            $digestSize = 32;
            $digestName = 'sha256';
            break;
          case Phar::SHA512:
            $digestSize = 64;
            $digestName = 'sha512';
            break;
          default:
            throw new PharException(
              'phar has a broken or unsupported signature'
            );
        }

        if ($signatureSize < 8 + $digestSize) {
          throw new PharException('phar has a broken signature');
        }

        $this->signature = substr($signature, 0, $digestSize);
        $computedSignature = $this->computeSignature(
          $digestName,
          $signatureSize
        );

        if (strcmp($computedSignature, $this->signature) !== 0) {
          throw new PharException('phar has a broken signature');
        }
      }
    }

    private function parseFileInfo(int $count, &$pos) {
      for ($i = 0; $i < $count; $i++) {
        $filename_len = $this->bytesToInt($pos, 4);
        $filename = $this->substr($pos, $filename_len);
        $filesize = $this->bytesToInt($pos, 4);
        $timestamp = $this->bytesToInt($pos, 4);
        $compressed_filesize = $this->bytesToInt($pos, 4);
        $crc32 = $this->bytesToInt($pos, 4);
        $flags = $this->bytesToInt($pos, 4);
        $metadata_len = $this->bytesToInt($pos, 4);
        $metadata = $this->bytesToInt($pos, $metadata_len);
        $this->fileInfo[$filename] = array(
          $filesize,
          $timestamp,
          $compressed_filesize ?: $filesize,
          $crc32,
          $flags,
          $metadata
        );
      }
    }

    private function computeSignature(
      string $algorithm,
      int $signatureSize
    ): string {
      $this->rewind();
      $context = hash_init($algorithm);
      $data = '';

      while (!$this->eof()) {
        $data .= $this->stream_get_contents(1024 * 1024);
        hash_update($context, substr($data, 0, -$signatureSize));
        $data = substr($data, -$signatureSize);
      }

      return hash_final($context, true);
    }

    private function bytesToInt(&$pos, int $len): int {
      $str = $this->stream_get_contents($len, $pos);
      if (strlen($str) < $len) {
        throw new PharException(
          "Corrupt phar, can't read $len bytes starting at offset $pos"
        );
      }
      $int = 0;
      for ($i = 0; $i < $len; ++$i) {
        $int |= ord($str[$i]) << (8*$i);
      }
      $pos += $len;
      return $int;
    }

    private function substr(&$pos, int $len) {
      $ret = $this->stream_get_contents($len, $pos);
      $pos += $len;
      return $ret;
    }

    public function extractAllTo(string $path) {
      throw new Exception('Not implemented yet');
    }

    public function addFile(string $path, string $archive_path): bool {
      throw new Exception('Not implemented yet');
    }
  }
}
