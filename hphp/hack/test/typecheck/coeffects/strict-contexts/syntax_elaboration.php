<?hh

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

function fully_qualified_but_wrong_ns()[
  \HH\Capabilities\AccessGlobals
]: void {}

function namespaceless_cap_bad()[AccessGlobals]: void {}

function fully_qualified_but_right_ns()[
  \HH\Contexts\write_props
]: void {}

function toplevel_lowercase_typename_bad()[int]: void {}
