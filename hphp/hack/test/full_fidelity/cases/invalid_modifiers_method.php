<?hh

abstract class C {
  // For now, methods are allowed to have all the possible modifiers, so this
  // verifies that we only get errors about the way they're being combined
  abstract final static private protected public async coroutine function f();

  abstract abstract public function g();
}
