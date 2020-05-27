<?hh // partial
<<__Sealed(Error::class, Exception::class)>>
interface Throwable {
  public function getMessage(): string;
  public function getCode(): int;
  <<__Rx, __MaybeMutable>>
  public function getFile(): string;
  <<__Rx, __MaybeMutable>>
  public function getLine(): int;
  public function getTrace(): \HH\Container;
  public function getTraceAsString(): string;
  <<__Rx, __MaybeMutable>>
  public function getPrevious(): ?Throwable;
  public function __toString(): string;
}
