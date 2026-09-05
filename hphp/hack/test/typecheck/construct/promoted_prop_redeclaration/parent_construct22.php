<?hh

interface IEntFoo {}
final class EntFooConfig implements IEntFoo {}

<<__ConsistentConstruct>>
abstract class EntFooBase<TConfig as IEntFoo> {
  public function __construct(protected TConfig $config) {}
}

// Re-promoting the inherited `config` property is rejected: HHVM fatals at
// runtime because the inherited type-hint bound is violated.
final class EntFoo extends EntFooBase<this::TConfig> {
  const type TConfig = EntFooConfig;
  public function __construct(protected this::TConfig $config) {
    parent::__construct($config);
  }
}

// OK: forwarding to the parent constructor without re-promoting.
final class EntFooOk extends EntFooBase<this::TConfig> {
  const type TConfig = EntFooConfig;
  public function __construct(this::TConfig $config) {
    parent::__construct($config);
  }
}

// Simple (non-generic) re-promotion of an inherited property is also rejected.
class Base {
  public function __construct(protected int $x) {}
}

class SubRepromote extends Base {
  public function __construct(protected int $x) {
    parent::__construct($x);
  }
}

// OK: forwarding without re-promoting.
class SubForward extends Base {
  public function __construct(int $x) {
    parent::__construct($x);
  }
}
