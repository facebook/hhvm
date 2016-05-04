<?php

namespace __SystemLib {
  use Exception;
  use Phar;
  use PharException;

  final class TarArchiveHandler extends ArchiveHandler {
    private Map<string, string> $contents = Map { };
    private Map<string, string> $symlinks = Map { };
    private $fp;

    public function __construct(
      string $path,
      bool $preventHaltTokenCheck = true
    ) {
      $this->path = $path;
      if (file_exists($path)) {
        $this->open($path);
        $this->parseTar();
        if (
          !$preventHaltTokenCheck &&
          strpos($this->stub, Phar::HALT_TOKEN) === false
        ) {
          throw new PharException(
            Phar::HALT_TOKEN.' must be declared in a phar'
          );
        }
      }
    }

    private function parseTar() {
      /* If you have GNU Tar installed, you should be able to find
       * the file format documentation (including header byte offsets) at:
       * - /usr/include/tar.h
       * - the tar info page (Top/Tar Internals/Standard)
       */

      $pos = 0;
      $next_file_name = null;
      while (!$this->eof()) {
        $header = $this->stream_get_contents(512);
        $pos += 512;
        // skip empty blocks
        if (!trim($header)) {
          continue;
        }

        $filename = trim(substr($header, 0, 100));
        if ($next_file_name) {
          $filename = $next_file_name;
          $next_file_name = null;
        }

        $size = octdec(substr($header, 124, 12));
        $timestamp = octdec(trim(substr($header, 136, 12)));
        $type = $header[156];

        // Hidden .phar directory should not appear in files listing
        if (strpos($filename, '.phar') === 0) {
          if ($filename == '.phar/stub.php') {
            $this->stub = $this->stream_get_contents($size);
          } else if ($filename == '.phar/alias.txt') {
            $this->alias = $this->stream_get_contents($size);
          }
        } else if (substr($filename, -1) !== '/') {
          $this->entries[$filename] = new ArchiveEntryStat(
            /* crc = */ null,
            $size,
            /* compressed size = */ $size,
            $timestamp
          );

          switch ($type) {
            case 'L':
              $next_file_name = trim($this->stream_get_contents($size));
              break;

            case '0':
            case "\0":
              $this->fileOffsets[$filename] = [$pos, $size];
              break;

            case '2':
              // Assuming this is from GNU Tar
              $target = trim(substr($header, 157, 100), "\0");
              $this->symlinks[$filename] = $target;
              break;

            case '5':
              // Directory, ignore
              break;

            default:
              throw new Exception("type $type is not implemented yet");
          }
        }
        $pos += $size;
        $this->seek($pos);
        if ($size % 512 !== 0) {
          $leftover = 512 - ($size % 512);
          $zeros = $this->stream_get_contents($leftover);
          if (strlen(trim($zeros)) != 0) {
            throw new Exception("Malformed tar. Padding isn't zeros. $zeros");
          }
          $pos += $leftover;
        }
      }
    }

    private function createFullPath(
      string $root,
      string $partial_path,
    ): string {
      $full_path = $root.'/'.$partial_path;
      $dir = dirname($full_path);
      if (!is_dir($dir)) {
        mkdir($dir, 0777, true);
      }
      return $full_path;
    }

    public function extractAllTo(string $root) {
      foreach ($this->fileOffsets as $path => list($offset, $size)) {
        $fp = fopen($this->createFullPath($root, $path), 'wb');
        while ($size) {
          $data = $this->stream_get_contents(min(1024, $size), $offset);
          fwrite($fp, $data);
          $size -= strlen($data);
          $offset += strlen($data);
        }
        fclose($fp);
      }

      // Intentional difference to PHP5: PHP5 just creates an empty
      // file.
      foreach ($this->symlinks as $path => $target) {
        symlink($target, $this->createFullPath($root, $path));
      }
    }

    public function addFile(string $path, string $archive_path) {
      if ($this->fp === null) {
        $this->fp = fopen($this->path, 'w');
      }

      if (strlen($archive_path) > 100) {
        $header = substr($archive_path, 0, 100);
        $header .= str_repeat("\0", 8); // mode
        $header .= str_repeat("\0", 8); // uid
        $header .= str_repeat("\0", 8); // gid
        $header .= str_pad(decoct(strlen($archive_path)), 11, '0', STR_PAD_LEFT)
          ."\0"; // length
        $header .= str_repeat("\0", 12); // mtime
        // Checksum in the middle...
        $header2 = 'L'; // type == long name
        $header2 .= str_repeat("\0", 100);

        // Checksum calculated as if the checksum field was spaces
        $to_checksum = $header.str_repeat(' ', 8).$header2;
        $sum = 0;
        foreach (unpack('C*', $to_checksum) as $char) {
          $sum += ord($char);
        }
        $checksum = str_pad(decoct($sum), 6, '0', STR_PAD_LEFT)."\0 ";
        fwrite($this->fp, str_pad($header.$checksum.$header2, 512, "\0"));
        $partial_block = strlen($archive_path) % 512;
        $padding = '';
        if ($partial_block !== 0) {
          $padding = str_repeat("\0", 512 - $partial_block);
        }
        fwrite($this->fp, $archive_path.$padding);
      }

      $stat = stat($path);
      $header = str_pad(substr($archive_path, 0, 100), 100, "\0");
      $header .= str_pad(decoct($stat['mode']), 7, '0', STR_PAD_LEFT)."\0";
      $header .= str_pad(decoct($stat['uid']), 7, '0', STR_PAD_LEFT)."\0";
      $header .= str_pad(decoct($stat['gid']), 7, '0', STR_PAD_LEFT)."\0";
      $header .= str_pad(decoct($stat['size']), 11, '0', STR_PAD_LEFT)."\0";
      $header .= str_pad(decoct($stat['mtime']), 11, '0', STR_PAD_LEFT)."\0";
      // Checksum in the middle...
      $header2 = '0'; // type == normal file
      $header2 .= str_repeat("\0", 100);

      // Checksum calculated as if the checksum field was spaces
      $to_checksum = $header.str_repeat(' ', 8).$header2;
      $sum = 0;
      foreach (unpack('C*', $to_checksum) as $char) {
        $sum += ord($char);
      }
      $checksum = str_pad(decoct($sum), 6, '0', STR_PAD_LEFT)."\0 ";
      fwrite($this->fp, str_pad($header.$checksum.$header2, 512, "\0"));
      $partial_block = $stat['size'] % 512;
      $padding = '';
      if ($partial_block !== 0) {
        $padding = str_repeat("\0", 512 - $partial_block);
      }
      fwrite($this->fp, file_get_contents($path).$padding);
      return true;
    }

    public function close(): void {
      if ($this->fp !== null) {
        fwrite($this->fp, str_repeat("\0", 1024));
        fclose($this->fp);
        $this->fp = null;
      }
    }
  }
}
