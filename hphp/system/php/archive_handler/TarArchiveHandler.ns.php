<?php

namespace __SystemLib {
  final class TarArchiveHandler extends ArchiveHandler {
    private Map<string, ArchiveEntryData> $entries = Map { };
    private Map<string, string> $contents = Map { };
    private Map<string, string> $symlinks = Map { };
    private string $path = '';
    private $fp = null;

    public function __construct(string $path) {
      $this->path = $path;
      $this->entries = Map { };

      if (file_exists($path)) {
        $this->readTar();
      }
    }

    private function readTar() {
      /* If you have GNU Tar installed, you should be able to find
       * the file format documentation (including header byte offsets) at:
       * - /usr/include/tar.h
       * - the tar info page (Top/Tar Internals/Standard)
       */

      $path = $this->path;
      $fp = fopen($path, 'rb');
      $data = fread($fp, 2);
      fclose($fp);

      if ($data === "\37\213") {
        $fp = gzopen($path, 'rb');
      } else if ($data === 'BZ') {
        $fp = bzopen($path, 'r');
      } else {
        $fp = fopen($path, 'rb');
      }

      $next_file_name = null;
      while (!feof($fp)) {
        $header = fread($fp, 512);
        // skip empty blocks
        if (!trim($header)) {
          continue;
        }

        $filename = trim(substr($header, 0, 100));
        if ($next_file_name) {
          $filename = $next_file_name;
          $next_file_name = null;
        }

        $len = octdec(substr($header, 124, 12));
        $timestamp = octdec(trim(substr($header, 136, 12)));
        $type = substr($header, 156, 1);
        $this->entries[$filename] = new ArchiveEntryStat(
          /* crc = */ null,
          $len,
          /* compressed size = */ null,
          $timestamp
        );
        $data = "";
        $read_len = 0;
        while ($read_len != $len) {
          $read_data = fread($fp, $len - $read_len);
          $data .= $read_data;
          $read_len += strlen($read_data);
        }

        switch ($type) {
          case 'L':
            $next_file_name = trim($data);
            break;

          case '0':
          case "\0":
            $this->contents[$filename] = $data;
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
            throw new \Exception("type $type is not implemented yet");
        }

        if ($len % 512 !== 0) {
          $leftover = 512 - ($len % 512);
          $zeros = fread($fp, $leftover);
          if (strlen(trim($zeros)) != 0) {
            throw new \Exception("Malformed tar. Padding isn't zeros. $zeros");
          }
        }
      }
    }

    public function getContentsList(): Map<string, ArchiveEntryStat> {
      return $this->entries;
    }

    public function read(string $path): ?string {
      if ($this->contents->contains($path)) {
        return $this->contents[$path];
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
      foreach ($this->contents as $path => $data) {
        file_put_contents($this->createFullPath($root, $path), $data);
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
