<?hh

interface I {
  public function i1(): void; // legal
  public async function i2(): Awaitable<void>; // error2046
}

abstract class C {
  public function c1(): void { } // legal
  public async function c2(): Awaitable<void> { } // legal
  public abstract async function c3(): Awaitable<void>; // error2046
}

trait T {
  public function t1(): void { } // legal
  public async function t2(): Awaitable<void> { } // legal
  public abstract async function t3(): Awaitable<void>; // error2046
}
