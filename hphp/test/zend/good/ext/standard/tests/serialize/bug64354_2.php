<?hh
class A {
    public function __wakeup() :mixed{
        throw new Exception("Failed");
    }
}
<<__EntryPoint>> function main(): void {
try {
    var_dump(unserialize('a:2:{i:0;O:1:"A":0:{}i:1;O:1:"B":0:{}}'));
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
