//// a.php
<?hh

new module A {}

//// b.php
<?hh

module A;

<<__ModuleLevelTrait>>
trait T {
  internal int $x = 42;
}

class C { use T; }

<<__EntryPoint>>
function main(): void {
  echo (new C())->x;
}
