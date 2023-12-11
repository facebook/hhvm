<?hh
class test_class {
   private function test_func1() :mixed{
        echo "test_func1\n";
   }
   protected function test_func2() :mixed{
        echo "test_func2\n";
   }
   static private function test_func3() :mixed{
        echo "test_func3\n";
   }
   static protected function test_func4() :mixed{
        echo "test_func4\n";
   }
   function test() :mixed{
     if (is_callable(vec[$this,'test_func1'])) {
         $this->test_func1();
     } else {
       echo "test_func1 isn't callable from inside\n";
     }
     if (is_callable(vec[$this,'test_func2'])) {
         $this->test_func2();
     } else {
       echo "test_func2 isn't callable from inside\n";
     }
     if (is_callable(vec['test_class','test_func3'])) {
         test_class::test_func3();
     } else {
       echo "test_func3 isn't callable from inside\n";
     }
     if (is_callable(vec['test_class','test_func4'])) {
         test_class::test_func4();
     } else {
       echo "test_func4 isn't callable from inside\n";
     }
   }
}

class foo extends test_class {
   function test() :mixed{
     if (is_callable(vec[$this,'test_func1'])) {
         $this->test_func1();
     } else {
       echo "test_func1 isn't callable from child\n";
     }
     if (is_callable(vec[$this,'test_func2'])) {
         $this->test_func2();
     } else {
       echo "test_func2 isn't callable from child\n";
     }
     if (is_callable(vec['test_class','test_func3'])) {
         test_class::test_func3();
     } else {
       echo "test_func3 isn't callable from child\n";
     }
     if (is_callable(vec['test_class','test_func4'])) {
         test_class::test_func4();
     } else {
       echo "test_func4 isn't callable from child\n";
     }
   }
}
<<__EntryPoint>> function main(): void {
$object = new test_class;
$object->test();
if (is_callable(vec[$object,'test_func1'])) {
    $object->test_func1();
} else {
  echo "test_func1 isn't callable from outside\n";
}
if (is_callable(vec[$object,'test_func2'])) {
    $object->test_func2();
} else {
  echo "test_func2 isn't callable from outside\n";
}
if (is_callable(vec['test_class','test_func3'])) {
  test_class::test_func3();
} else {
  echo "test_func3 isn't callable from outside\n";
}
if (is_callable(vec['test_class','test_func4'])) {
  test_class::test_func4();
} else {
  echo "test_func4 isn't callable from outside\n";
}
$object = new foo();
$object->test();
}
