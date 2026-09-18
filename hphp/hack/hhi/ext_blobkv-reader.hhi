<?hh

namespace HH\BlobKV;

final class Reader {
  public function __construct(string $path);
  public function read(string $key): ?string;
}
