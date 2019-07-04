<?hh

include __DIR__."/builtin_extensions.inc";

class A_SQLite3 extends SQLite3 {
  public $___x;
}
<<__EntryPoint>> function main(): void {
test("SQLite3", ":memory:");
}
