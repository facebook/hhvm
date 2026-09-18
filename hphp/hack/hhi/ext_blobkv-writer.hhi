<?hh

namespace HH\BlobKV;

final class Writer {
  public function __construct(string $path);
  public function write(string $key, string $value): void;
  public function finalize(): void;
}
