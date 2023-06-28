<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.


// tests FCallFuncD
<<__DynamicallyCallable>>
function hello() :mixed{
  echo "hello";
  return 1;
}

// tests FCallFunc
function doCall($f) :mixed{
  echo "doCall(";
  if ($f()) {
    echo ") -- doCall got true\n";
  } else {
    echo ") -- doCall got false\n";
  }
}

class C {
  public function method1() :mixed{
    $x = 1;
    echo "method1\n";
    if ($x) {
      doCall("hello");
      return hello();
    }
    return 0;
  }
  public function doPr($x) :mixed{
    echo $x;
    echo "\n";
    if ($x) {
      return 1;
    }
    return 2;
  }
  public function foo() :mixed{
    echo "foo\n";
    if ($this->method1()) {
      if ($this->doPr(true)) {
        if ($this->doPr(49)) {
          return 1;
        }
      }
      return 1;
    }
    return 2;
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
