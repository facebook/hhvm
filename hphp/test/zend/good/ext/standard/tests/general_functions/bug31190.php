<?hh

class test {
  <<__DynamicallyCallable>> function throwException() { throw new Exception("Hello World!\n"); }
}
<<__EntryPoint>> function main(): void {
$array = varray[new test(), 'throwException'];
try {
     call_user_func($array, 1, 2);
} catch (Exception $e) {
     echo $e->getMessage();
}

try {
     call_user_func_array($array, varray[1, 2]);
} catch (Exception $e) {
     echo $e->getMessage();
}
}
