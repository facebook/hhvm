<?hh

class A_SQLite3 extends SQLite3 {
  public $___x;
}
<<__EntryPoint>> function main(): void {
  include __DIR__."/builtin_extensions.inc";
  test("SQLite3", ":memory:");
}
