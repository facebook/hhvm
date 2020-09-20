<?hh // partial

<<__Sealed(Error::class, Exception::class)>>
interface Throwable {
  public function getMessage(): string;
  <<__Pure, __MaybeMutable>>
  public function getCode(): int;
  <<__Pure, __MaybeMutable>>
  public function getFile(): string;
  <<__Pure, __MaybeMutable>>
  public function getLine(): int;
  <<__Pure, __MaybeMutable>>
  public function getTrace(): \HH\Container;
  <<__Pure, __MaybeMutable>>
  public function getTraceAsString(): string;
  <<__Pure, __MaybeMutable>>
  public function getPrevious(): ?Throwable;
  public function __toString(): string;
}
