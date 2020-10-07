<?hh

class A {
  public async function foo() : Awaitable<bool> {
    return true;
  }
}

class B {
  public function foo() : bool {
    return true;
  }
}


async function test() : Awaitable<bool> {
  $a = new A();
  $lambda = async ($maybe_a) ==> { return await $maybe_a->foo(); };
  return await $lambda($a);
}

async function bad_test() : Awaitable<bool> {
  $b = new B();
  $lambda = async ($not_b) ==> { return await $not_b->foo(); };
  return await $lambda($b);
}
