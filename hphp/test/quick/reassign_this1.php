<?hh
class Foo { public $x; }

<<__EntryPoint>> function main(): void {
$this = new Foo();
echo $this->x;
}
