<?hh // strict

namespace {
  class IExistInTheRootNamespace {}

  final xhp class foo:bar {
    public function __construct(mixed ...$args) {
      \var_dump(vec[__LINE__, new IExistInTheRootNamespace()]);
      \var_dump(vec[__LINE__, $this, \get_class($this)]);
    }
  }
}

namespace HerpDerp {

  final xhp class foo:bar {
    public function __construct(mixed ...$args) {
      \var_dump(vec[__LINE__, new \IExistInTheRootNamespace()]);
      \var_dump(vec[__LINE__, $this, \get_class($this)]);
    }
  }

  <<__EntryPoint>>
  function test_instantiation(): void {
   $_ = <foo:bar />;
   $_ = <:foo:bar />;
  }

}
