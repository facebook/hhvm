<?hh
class C {
  public static function log(Exception $exception) {
    $msg = get_class($exception).': '.$exception->getMessage();
    var_dump($msg);
  }
  public static function setup() {
    set_exception_handler(array(__CLASS__, 'log'));
  }
}
<<__EntryPoint>> function main(): void {
C::setup();
throw new Exception('test');
}
