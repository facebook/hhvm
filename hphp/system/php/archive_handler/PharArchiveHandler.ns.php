<?php

namespace __SystemLib {
  use Phar;
  use PharException;

  final class PharArchiveHandler extends ArchiveHandler {
    private array<string, array> $fileInfo = array();
    private int $archiveFlags;
    public function __construct(string $path, $preventHaltTokenCheck) {
      $this->fp = fopen($path, 'rb');
      $data = file_get_contents($path);

      $pos = strpos($data, Phar::HALT_TOKEN);
      if ($pos === false && !$preventHaltTokenCheck) {
        throw new PharException(Phar::HALT_TOKEN.' must be declared in a phar');
      }
      $this->stub = substr($data, 0, $pos);

      $pos += strlen(Phar::HALT_TOKEN);
      // *sigh*. We have to allow whitespace then ending the file
      // before we start the manifest
      while ($data[$pos] == ' ') {
        $pos += 1;
      }
      if ($data[$pos] == '?' && $data[$pos+1] == '>') {
        $pos += 2;
      }
      while ($data[$pos] == "\r") {
        $pos += 1;
      }
      while ($data[$pos] == "\n") {
        $pos += 1;
      }

      $this->parsePhar($data, $pos);
    }

    private function parsePhar($data, &$pos) {
      $start = $pos;
      $len = self::bytesToInt($data, $pos, 4);
      $count = self::bytesToInt($data, $pos, 4);
      $this->apiVersion = self::bytesToInt($data, $pos, 2);
      $this->archiveFlags = self::bytesToInt($data, $pos, 4);
      $alias_len = self::bytesToInt($data, $pos, 4);
      $this->alias = self::substr($data, $pos, $alias_len);
      $metadata_len = self::bytesToInt($data, $pos, 4);
      $this->metadata = unserialize(
        self::substr($data, $pos, $metadata_len)
      );
      $this->parseFileInfo($data, $count, $pos);
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

      // Try to see if there is a signature
      if ($this->archiveFlags & Phar::SIGNATURE) {
        if (strlen($data) < 8 || substr($data, -4) !== 'GBMB') {
          // Not even the GBMB and the flags?
          throw new PharException('phar has a broken signature');
        }

        $pos = strlen($data) - 8;
        $signatureFlags = self::bytesToInt($data, $pos, 4);
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
            throw new PharException('phar has a broken or unsupported signature');
        }

        if (strlen($data) < 8 + $digestSize) {
          throw new PharException('phar has a broken signature');
        }

        $pos -= 4;
        $signatureStart = $pos - $digestSize;
        $this->signature = substr($data, $signatureStart, $digestSize);
        $actualHash = self::verifyHash($data, $digestName, $signatureStart);

        if ($actualHash !== $this->signature) {
          throw new PharException('phar has a broken signature');
        }
      }
    }

    private function parseFileInfo(string $str, int $count, &$pos) {
      for ($i = 0; $i < $count; $i++) {
        $filename_len = self::bytesToInt($str, $pos, 4);
        $filename = self::substr($str, $pos, $filename_len);
        $filesize = self::bytesToInt($str, $pos, 4);
        $timestamp = self::bytesToInt($str, $pos, 4);
        $compressed_filesize = self::bytesToInt($str, $pos, 4);
        $crc32 = self::bytesToInt($str, $pos, 4);
        $flags = self::bytesToInt($str, $pos, 4);
        $metadata_len = self::bytesToInt($str, $pos, 4);
        $metadata = self::bytesToInt($str, $pos, $metadata_len);
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

    private static function verifyHash($str, $algorithm, $signatureOffset) {
      return hash($algorithm, substr($str, 0, $signatureOffset), true);
    }

    private static function bytesToInt($str, &$pos, $len) {
      if (strlen($str) < $pos + $len) {
        throw new PharException(
          "Corrupt phar, can't read $len bytes starting at offset $pos"
        );
      }
      $int = 0;
      for ($i = 0; $i < $len; ++$i) {
        $int |= ord($str[$pos++]) << (8*$i);
      }
      return $int;
    }

    private static function substr($str, &$pos, $len) {
      $ret = substr($str, $pos, $len);
      $pos += $len;
      return $ret;
    }

    public function close(): void {
      //TODO
    }

    public function read(string $path): ?string {
      $data = $this->za->getFromName($path);
      if ($data === false) {
        return null;
      }
      return $data;
    }

    public function extractAllTo(string $path) {
      //TODO
    }

    public function addFile(string $path, string $archive_path): bool {
      //TODO
    }

    // Custom methods used by Phar class internally

    public function getFileContents(string $filename): string {
      if (!isset($this->fileOffsets[$filename])) {
        throw new PharException("No $filename in phar");
      }
      list($offset, $size) = $this->fileOffsets[$filename];
      if ($size == 0) {
        return '';
      }
      fseek($this->fp, $offset);
      return fread($this->fp, $size);
    }
  }
}
