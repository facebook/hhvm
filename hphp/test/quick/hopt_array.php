<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class C {
  public function foo() :mixed{
    echo "foo\n";
    $x = vec[];
    if ($x) {
      print_r($x);
    } else {
      print_r($x);
    }
  }
}
<<__EntryPoint>> function main(): void {
echo "Starting\n";
$c = new C;
$val = $c->foo();
echo $val;
echo "\n";
echo "Done\n";
}
