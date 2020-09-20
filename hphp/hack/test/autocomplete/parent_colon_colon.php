<?hh

class Ac1 {
  public function fun_instance(): void { }
  public static function fun_method(): void { }
}

class Bc1 extends Ac1 {
  public function test() { parent::fun_AUTO332 }
}
