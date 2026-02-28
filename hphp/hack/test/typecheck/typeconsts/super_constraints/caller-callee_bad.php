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
  public function uncallable_on_base(Base $base, this::TC $cap): void {
    $base->do($cap); // ERROR
  }
  public function withCap((function(this::TC): void) $do): void {
    $do(new CanReadImpl());
  }
}
class CanReadImpl implements CanRead {}

abstract class WriteAndAppend extends Base {
  abstract const type TC super CanWrite;
  public function cannot_write_with_append_only(CanAppend $cap): void {
    $this->do($cap);  // ERROR: CanAppend  <:/  >CanWrite
    (new AppendOnly())->withCap($canAppendOnly ==> $this->do($canAppendOnly));  // ERROR
    // >CanAppend  </:  >:CanWrite  (>CanAppend may be a supertype that isn't CanWrite)

    $this->withCap($canWriteAndAppend ==> (new AppendOnly())->do($canWriteAndAppend)); // ERROR
    // >CanWrite  </:  >CanAppend
  }
  <<__Override>>
  public function do(this::TC $cap): void {
    $this->cannot_write_with_append_only($cap);  // ERROR: >CanWrite  </:  CanAppend
  }
}

class AppendOnly extends Base {
  const type TC = CanAppend;
  public function cannot_append_with_append_super_sub(): void {
    $this->withCap($canAppendOnly ==> $this->do($canAppendOnly));
    // >CanAppend  <:  >CanAppend
  }
  public function cannot_write_with_append_super_sub(WriteAndAppend $wa): void {
    $this->withCap($canAppendOnly ==> $wa->do($canAppendOnly));  // ERROR
  }
  public function withCap((function(this::TC): void) $do): void {
    $do(new CanAppendImpl());
  }
}
class CanAppendImpl implements CanAppend {}

function cannot_write_with_append_only_fixed(WriteAndAppend $wa, CanAppend $cap): void {
  $wa->do($cap);  // ERROR: CanAppend  </: >CanWrite  (a type between them may exist)
}
