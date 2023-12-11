<?hh

class test {
  <<__DynamicallyCallable>> function throwException() :mixed{ throw new Exception("Hello World!\n"); }
}
<<__EntryPoint>> function main(): void {
$array = vec[new test(), 'throwException'];
try {
     call_user_func($array, 1, 2);
} catch (Exception $e) {
     echo $e->getMessage();
}

try {
     call_user_func_array($array, vec[1, 2]);
} catch (Exception $e) {
     echo $e->getMessage();
}
}
