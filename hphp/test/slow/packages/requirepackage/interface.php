<?hh

// package foo

interface I {
  <<__RequirePackage('bar')>>
  public function f(): void;

  <<__RequirePackage('bat')>>
  public function g(): void;
}

class C implements I {
  <<__RequirePackage('bar')>>
  public function f(): void {
    echo "can see bar\n";
  }

  <<__RequirePackage('bat')>>
  public function g(): void {
    echo "can see bat\n";
  }
}

function call_bar(I $i): void {
  $i->f();
}

function call_bat(I $i): void {
  $i->g();
}

<<__EntryPoint>>
function main_interface(): void {
  $c = new C();
  call_bar($c);
  call_bat($c);
}
