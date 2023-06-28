<?hh

abstract final class TestStatics {
  public static $a =1;
}

function Test()
:mixed{
    echo TestStatics::$a . " ";
    TestStatics::$a++;
    if(TestStatics::$a<10) Test();
}
<<__EntryPoint>> function main(): void {
Test();
}
