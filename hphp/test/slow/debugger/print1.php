<?hh

class C {
  public function __toDebugDisplay() :mixed{
    return "some custom string";
  }
}

class D {
  public function __toDebugDisplay() :mixed{
    return dict['c'=>new C()];
  }
}

<<__EntryPoint>> function main() :mixed{
$a = dict[];
for ($i = 0; $i < 20; $i++) { $a[$i] = $i; }
$s = "hello";
$ch = curl_init();

$c = new C();
$d = new D();
1;
}
