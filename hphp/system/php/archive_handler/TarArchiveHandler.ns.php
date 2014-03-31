<?php

namespace __SystemLib {
  final class TarArchiveHandler extends ArchiveHandler {
    private Map<string, ArchiveEntryData> $entries;
    private string $path = '';

    public function __construct(string $path) {
      $this->path = $path;
      $this->entries = Map { };

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
            $next_file_name = $data;
            break;

          case '0':
          case "\0":
            $this->contents[$filename] = $data;
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

    public function extractAllTo(string $root): bool {
      foreach ($this->contents as $path => $data) {
        $path = $root.'/'.$path;
        $dir = dirname($path);
        if (!is_dir($dir)) {
          mkdir($dir, 0777, true);
        }
        file_put_contents($path, $data);
      }
    }

    public function addFile(string $path, string $archivePath) {
      throw new Exception("TarArchiveHandler::addFile is not implemented");
    }

    public function close(): void {
    }
  }
}
