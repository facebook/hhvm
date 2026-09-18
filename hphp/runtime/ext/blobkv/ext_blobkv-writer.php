<?hh

namespace HH\BlobKV;

<<__NativeData>>
final class Writer {
  <<__Native>>
  public function __construct(string $path): void;

  <<__Native>>
  public function write(string $key, string $value): void;

  <<__Native>>
  public function finalize(): void;
}
