<?hh

class HasGeneric<T as ?arraykey> {

  public async function foo(T $t): Awaitable<arraykey> {
    if ($t is null) {
      throw new Exception();
    }
    return $t;
  }
}
