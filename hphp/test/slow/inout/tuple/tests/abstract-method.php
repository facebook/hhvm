<?hh

interface I {
  public function foo(int &$x): Awaitable<void>;
}

class C implements I {
  public async function foo(int &$x): Awaitable<void> {}
}

abstract class B {
  public abstract function foo(int &$x): Awaitable<void>;
}

class D extends B {
  public async function foo(int &$x): Awaitable<void> {}
}

echo "Done.\n";
