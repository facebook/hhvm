<?hh

async function fun() {}

abstract class C {
  abstract const FOO;
  public static $x1;
  static public $x2;

  protected static abstract async function bar1();
  protected abstract static async function bar2();
  static protected abstract async function bar3();
  static abstract protected async function bar4();
  abstract static protected async function bar5();
  abstract protected static async function bar6();
}
