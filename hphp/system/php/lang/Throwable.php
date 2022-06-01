<?hh // partial

<<__Sealed(
  /* HH_FIXME[2049] */
  Error::class,
  /* HH_FIXME[2049] */
  Exception::class,
)>>
interface Throwable {
  public function getMessage(): string;
  public function getCode()[]: int;
  public function getFile()[]: string;
  public function getLine()[]: int;
  public function getTrace()[]: \HH\Container<mixed>;
  public function getTraceAsString()[]: string;
  public function getPrevious()[]: ?Throwable;
  public function __toString(): string;
}
