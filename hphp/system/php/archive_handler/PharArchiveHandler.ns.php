<?php

namespace __SystemLib {
  use Phar;
  use PharException;

  final class PharArchiveHandler extends ArchiveHandler {
    private array<string, array> $fileInfo = array();
    private int $archiveFlags;
    private ?resource $stream;
    public function __construct(
      string $path,
      bool $preventHaltTokenCheck = true
    ) {
      $stream = fopen($path, 'rb');
      $this->stream = $stream;
      // We'll not get 0 as position because of `<?php` at the beginning
      $pos = self::findToken($stream, Phar::HALT_TOKEN) ?: 0;
      if ($pos) {
        $this->stub = stream_get_contents($stream, $pos, 0);
        $pos += strlen(Phar::HALT_TOKEN);
        // *sigh*. We have to allow whitespace then ending the file
        // before we start the manifest
        while (stream_get_contents($stream, 1, $pos) == ' ') {
          $pos += 1;
        }
        if (
          stream_get_contents($stream, 1, $pos) == '?' &&
          stream_get_contents($stream, 1, $pos + 1) == '>'
        ) {
          $pos += 2;
        }
        while (stream_get_contents($stream, 1, $pos) == "\r") {
          $pos += 1;
        }
        while (stream_get_contents($stream, 1, $pos) == "\n") {
          $pos += 1;
        }
      } else if (!$preventHaltTokenCheck) {
        throw new PharException(Phar::HALT_TOKEN.' must be declared in a phar');
      }

      $this->parsePhar($stream, $pos);
    }

    private static function findToken(resource $stream, string $token): ?int {
      $offset = 0;
      $prev_data = '';
      $next_data = '';
      do {
        $offset += strlen($prev_data);
        $prev_data = $next_data;
        $next_data = fread($stream, 1024);
        $pos = strpos($prev_data.$next_data, $token);
        if ($pos !== false) {
          return $offset + $pos;
        }
      } while (!feof($stream));
      return null;
    }

    private function parsePhar(resource $stream, &$pos) {
      $start = $pos;
      $len = self::bytesToInt($stream, $pos, 4);
      $count = self::bytesToInt($stream, $pos, 4);
      $this->apiVersion = self::bytesToInt($stream, $pos, 2);
      $this->archiveFlags = self::bytesToInt($stream, $pos, 4);
      $alias_len = self::bytesToInt($stream, $pos, 4);
      $this->alias = self::substr($stream, $pos, $alias_len);
      $metadata_len = self::bytesToInt($stream, $pos, 4);
      $this->metadata = unserialize(
        self::substr($stream, $pos, $metadata_len)
      );
      $this->parseFileInfo($stream, $count, $pos);
      if ($pos != $start + $len + 4) {
        throw new PharException(
          "Malformed manifest. Expected $len bytes, got $pos"
        );
      }
      foreach ($this->fileInfo as $filename => $info) {
        $this->fileOffsets[$filename] = array($pos, $info[2]);
        $pos += $info[2];
        $this->entries[$filename] = new ArchiveEntryStat(
          $info[3],
          $info[0],
          $info[2],
          $info[1]
        );
      }

      $signatureStart = $pos;
      $signature = stream_get_contents($stream, -1, $pos);
      $signatureSize = strlen($signature);

      // Try to see if there is a signature
      if ($this->archiveFlags & Phar::SIGNATURE) {
        if (substr($signature, -4) !== 'GBMB') {
          // Not even the GBMB and the flags?
          throw new PharException('phar has a broken signature');
        }

        $pos = $signatureStart + $signatureSize - 8;
        $signatureFlags = self::bytesToInt($stream, $pos, 4);
        switch ($signatureFlags) {
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
        $computedSignature = self::computeSignature(
          $stream,
          $digestName,
          $signatureSize
        );

        if (strcmp($computedSignature, $this->signature) !== 0) {
          throw new PharException('phar has a broken signature');
        }
      }
    }

    private function parseFileInfo(resource $stream, int $count, &$pos) {
      for ($i = 0; $i < $count; $i++) {
        $filename_len = self::bytesToInt($stream, $pos, 4);
        $filename = self::substr($stream, $pos, $filename_len);
        $filesize = self::bytesToInt($stream, $pos, 4);
        $timestamp = self::bytesToInt($stream, $pos, 4);
        $compressed_filesize = self::bytesToInt($stream, $pos, 4);
        $crc32 = self::bytesToInt($stream, $pos, 4);
        $flags = self::bytesToInt($stream, $pos, 4);
        $metadata_len = self::bytesToInt($stream, $pos, 4);
        $metadata = self::bytesToInt($stream, $pos, $metadata_len);
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

    private static function computeSignature(
      resource $stream,
      string $algorithm,
      int $signatureSize
    ) {
      rewind($stream);
      $context = hash_init($algorithm);
      $data = '';

      while (!feof($stream)) {
        $data .= fread($stream, 1024 * 1024);
        hash_update($context, substr($data, 0, -$signatureSize));
        $data = substr($data, -$signatureSize);
      }

      return hash_final($context, true);
    }

    private static function bytesToInt(resource $stream, &$pos, int $len) {
      $str = stream_get_contents($stream, $len, $pos);
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

    private static function substr(resource $stream, &$pos, int $len) {
      $ret = stream_get_contents($stream, $len, $pos);
      $pos += $len;
      return $ret;
    }

    public function close(): void {
      if ($this->stream) {
        fclose($this->stream);
        $this->stream = null;
      }
    }

    public function getStream(string $path): resource {
      if (!isset($this->fileOffsets[$path])) {
        throw new PharException("No $path in phar");
      }
      list($offset, $size) = $this->fileOffsets[$path];
      if ($size == 0) {
        return fopen('php://temp', 'w+b');
      }
      $stream = fopen('php://temp', 'w+b');//TODO stream slice needed here
      fwrite($stream, stream_get_contents($this->stream, $size, $offset));
      return $stream;
    }

    public function extractAllTo(string $path) {
      //TODO
    }

    public function addFile(string $path, string $archive_path): bool {
      //TODO
    }
  }
}
