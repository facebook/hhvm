<?hh

class C {
  public function __toDebugDisplay() {
    return "some custom string";
  }
}

class D {
  public function __toDebugDisplay() {
    return darray['c'=>new C()];
  }
}

<<__EntryPoint>> function main() {
$a = darray[];
for ($i = 0; $i < 20; $i++) { $a[$i] = $i; }
$s = "hello";
$ch = curl_init();

$c = new C();
$d = new D();
1;
}
