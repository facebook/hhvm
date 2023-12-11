<?hh
class C {
  <<__DynamicallyCallable>> public static function log(Exception $exception) :mixed{
    $msg = get_class($exception).': '.$exception->getMessage();
    var_dump($msg);
  }
  public static function setup() :mixed{
    set_exception_handler(vec[__CLASS__, 'log']);
  }
}
<<__EntryPoint>> function main(): void {
C::setup();
throw new Exception('test');
}
