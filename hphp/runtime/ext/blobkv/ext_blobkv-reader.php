<?hh

namespace HH\BlobKV;

<<__NativeData>>
final class Reader {
  <<__Native>>
  public function __construct(string $path): void;

  <<__Native>>
  public function read(string $key): ?string;
}
