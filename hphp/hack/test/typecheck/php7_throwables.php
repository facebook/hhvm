<?hh // strict


class Foo implements Throwable {
  public function getMessage(): string {
    return __FUNCTION__;
  }
  public function getCode(): int {
    return __LINE__;
  }
  public function getLine(): int {
    return __LINE__;
  }
  public function getFile(): string {
    return __FILE__;
  }
  public function getTrace(): array<mixed> {
    return [];
  }
  public function getTraceAsString(): string {
    return 'nope';
  }
  public function getPrevious(): ?Throwable {
    return null;
  }
  public function __toString(): string {
    return __FUNCTION__;
  }
}

function catches(): void {
  try {
  } catch (Exception $_) {
  }
  try {
  } catch (Throwable $_) {
  }
  try {
  } catch (Error $_) {
  }
  try {
  } catch (Foo $_) {
  }
}

function throws(): void {
  throw new Exception();
  throw new Error();
  throw new TypeError();
  throw new Foo();
}
