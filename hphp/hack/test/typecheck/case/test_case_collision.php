<?hh // strict

class A {
  public static async function foo(int $x, int $y): Awaitable<int> {
    return $x + $y;
  }

  public static async function FOO(int $x, int $y): Awaitable<int> {
    return $x - $y;
  }
}
