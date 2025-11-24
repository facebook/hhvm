<?hh

// package foo

abstract class AC {
  <<__RequirePackage('bar')>>
  abstract public function f(): void;

  <<__RequirePackage('bat')>>
  abstract public function g(): void;
}

class CC extends AC {
  <<__RequirePackage('bar')>>
  public function f(): void {
    echo "can see bar\n";
  }

  <<__RequirePackage('bat')>>
  public function g(): void {
    echo "can see bat\n";
  }
}

function call_bar_2(AC $i): void {
  $i->f();
}

function call_bat_2(AC $i): void {
  $i->g();
}

<<__EntryPoint>>
function main_abstract(): void {
  $c = new CC();
  call_bar_2($c);
  call_bat_2($c);
}
