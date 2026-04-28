# Contexts & Subtyping

Capabilities are contravariant: a closure that requires *fewer* capabilities may be passed where a closure requiring *more* capabilities is expected.

```hack
function requires_globals_arg((function()[globals]: void) $f): void {
  $f();
}

function caller(): void {
  // ok: passing a function that requires fewer capabilities
  requires_globals_arg(()[read_globals] ==> {});
  // ok: passing a function that requires no capabilities
  requires_globals_arg(()[] ==> {});
}
```

This has the standard implication on inheritance hierarchies. A child class may override a method with a context that requires the same or fewer capabilities than the parent.

```hack
class Parent_ {
  public function maybeGlobals()[globals]: void {} // {AccessGlobals}
  public function maybePure(): void {}             // defaults
}

class Mid extends Parent_ {
  // ok: {ReadGlobals} ⊆ {AccessGlobals}
  public function maybeGlobals()[read_globals]: void {}
  // ok: {WriteProperty} ⊆ defaults
  public function maybePure()[write_props]: void {}
}

class Child extends Mid {
  // ok: {} ⊆ {ReadGlobals}
  public function maybeGlobals()[]: void {}
  // ok: {} ⊆ {WriteProperty}
  public function maybePure()[]: void {}
}
```

### Capability subtyping

There may also exist a subtyping relationship between capabilities themselves. When a capability B is a subtype of capability A, any function or operation that requires A may be called from a context providing B.

For example, `AccessGlobals` includes `ReadGlobals`, so a function with the `globals` context can call functions requiring `read_globals`:

```hack
function requires_read_globals()[read_globals]: void {}

function has_globals()[globals]: void {
  // ok: AccessGlobals includes ReadGlobals
  requires_read_globals();
}
```
