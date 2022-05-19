<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

interface Can {}

interface CanRead extends Can {}

interface CanAppend extends Can {}
interface CanWrite extends CanAppend {}

interface CanIO extends CanRead, CanWrite {}

abstract class Base {
  abstract const type TC super CanIO;

  public function do(this::TC $cap): void {}
  public abstract function withCap((function(this::TC): void) $do): void;
}

class ReadOnly extends Base {
  const type TC = CanRead;
  public function withCap((function(this::TC): void) $do): void {
    $do(new CanReadImpl());
  }
}
class CanReadImpl implements CanRead {}

abstract class WriteAndAppend extends Base {
  abstract const type TC super CanWrite;

  public function callable_with_write(CanWrite $cap): void {
    $this->do($cap); // OK:  CanWrite  <:  >CanWrite=this::C
  }
}
class AppendOnly extends Base {
  const type TC = CanAppend;
  public function can_do_with_append(CanAppend $cap): void {
    $this->do($cap);  // OK: CanAppend  <:  >CanAppend=this::C
  }
  public function cannot_append_with_append_super(this::TC $cap): void {
    $this->do($cap);  // OK: this::TC  <:  this::TC
  }
  public function can_append_with_append_poly2(): void {
    $this->withCap($canAppendOnly ==> $this->do($canAppendOnly));
    // OK: >CanAppend  <:  >CanAppend
  }
  public function withCap((function(this::TC): void) $do): void {
    $do(new CanAppendImpl());
  }
}
class CanAppendImpl implements CanAppend {}

function can_append_with_write(CanWrite $cap): void {
  (new AppendOnly())->do($cap);  // OK: CanWrite  <:  >CanAppend
}
