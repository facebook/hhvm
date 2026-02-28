<?hh

class SecretResource {
  public function __construct(private string $secret_path): void {}

  public async function doSomething(): Awaitable<string> {
    return "lolololol";
  }

  public function close(): void {}
}

class Handle<TScope> {
  public function __construct(private SecretResource $resource) {}

  public async function doSomething(): Awaitable<string> {
    return await $this->resource->doSomething();
  }

  public function close(): void {
    $this->resource->close();
  }
}

function withSecretResourceUnsafe<TScope, T>(
  string $path,
  (function(Handle<TScope>): T) $f,
): T {
  $handle = new Handle(new SecretResource($path));
  try {
    return $f($handle);
  } finally {
    $handle->close();
  }
}

function withSecretResourceSafe<T>(
  string $path,
  (function<TScope>(Handle<TScope>): T) $f,
): T {
  $handle = new Handle(new SecretResource($path));
  try {
    return $f($handle);
  } finally {
    $handle->close();
  }
}

function tryAndGetTheHandle(): void {
  $escaping_handle =
    withSecretResourceUnsafe("tell_nobody.txt", $handle ==> $handle);

  $nope = withSecretResourceSafe(
    "tell_nobody.txt",
    function<TScope>(Handle<TScope> $handle): Handle<TScope> ==> $handle,
  );
}
