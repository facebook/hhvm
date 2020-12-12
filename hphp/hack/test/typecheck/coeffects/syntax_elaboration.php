<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class Obj {
  public function implicit_defaults_ctx(): void {
    $this->explicit_defaults_ctx();
  }

  public function explicit_defaults_ctx()[defaults]: void {
    $this->implicit_defaults_ctx();
  }

  public function fully_qualified_cap_good()[
    \HH\Capabilities\WriteProperty
  ]: void {}

  public function namespaceless_cap_bad()[WriteProperty]: void {}
}

function fun_implicit_defaults_ctx(): void {
  fun_explicit_defaults_ctx();
}

function fun_explicit_defaults_ctx()[defaults]: void {
  fun_implicit_defaults_ctx();
}

function fully_qualified_cap_good()[
  \HH\Capabilities\AccessStaticVariable
]: void {}

function namespaceless_cap_bad()[AccessStaticVariable]: void {}
