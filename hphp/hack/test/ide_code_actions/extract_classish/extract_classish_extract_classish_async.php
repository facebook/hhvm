<?hh

class A {
  // Bug: extract interface method closing brace will be commented-out
  /*range-start*/
  public async function foo(): Awaitable<void> {}
  /*range-end*/
}
