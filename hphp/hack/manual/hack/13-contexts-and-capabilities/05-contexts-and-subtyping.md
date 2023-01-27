**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

Capabilities are contravariant.

This implies that a closure that requires a set of capabilities S<sub>a</sub> may be passed where the expected type is a function that requires S<sub>b</sub> as long as S<sub>a</sub> ⊆ S<sub>b</sub>.

For the following example, assume that the default context includes *at least* {Rand, IO}

```hack no-extract
function requires_rand_io_arg((function()[rand, io]: void) $f): void {
  $f();
}

function caller(): void {
  // passing a function that requires fewer capabilities
  requires_rand_io_arg(()[rand] ==> {/* some fn body */});
  // passing a function that requires no capabilities
  requires_rand_io_arg(()[] ==> {/* some fn body */});
}
```

Additionally, this has the standard implication on inheritance hierarchies. Note that if S<sub>a</sub> ⊆ S<sub>b</sub> it is the case that S<sub>b</sub> is a subtype of S<sub>a</sub>.

For the following, assume the default set contains {IO, Rand, Throws<mixed>}.

```hack no-extract
class Parent_ {
  public function maybeRand()[rand]: void {/* some fn body */} // {Rand}
  public function maybePure(): void {/* some fn body */} // {Throws<mixed>, IO, Rand}
}

class Mid extends Parent_ {
  public function maybeRand()[rand]: void {/* some fn body */} // {Rand} -> fine {Rand} ⊆ {Rand}
  public function maybePure()[io]: void {/* some fn body */} // {IO} -> fine {IO} ⊆ {Throws<mixed>, IO, Rand}
}

class Child extends Mid {
  public function maybeRand()[]: void {/* some fn body */} // {} -> fine {} ⊆ {Rand}
  public function maybePure()[]: void {/* some fn body */} // {} -> fine {} ⊆ {IO}
}
```

### Capability subtyping

In reality, there may also exist a subtyping relationship between capabilities. Thus, whenever some capability B that is subtype of capability A is available, any function or operation that requires A may be called or performed, respectively. This works identically to standard type subtyping.

For the following, assume that the following contexts and capabilities exist: Rand, ReadFile <: Rand, rand: {Rand}, readfile: {Readfile}

```hack no-extract
function requires_rand()[rand]: void {/* some fn body */}

function has_readfile()[readfile]: void {
  requires_rand(); // fine {readfile} ⊆ {Rand} since readfile <: rand
}
```
