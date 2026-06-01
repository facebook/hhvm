//// warning_cc_08.php
<?hh

// package pkg2
class C2FA {}

class N08A {} // this extra class silences the filename linter

//// bar_fa.php
<?hh

// package pkg1
class MyFileAttr implements \HH\FileAttribute {
  public function __construct(private classname<mixed> $cls) {}
}

<<file:MyFileAttr(C2FA::class)>> // lint should NOT fire on file attribute

class C1FA {
  public function foo(): void {
    $c = C2FA::class;  // sanity check: linter should fire here
  }
}

class N08B {} // this extra class silences the filename linter
