<?hh

trait T {
  <<__DynamicallyCallable>>
  static function foo() :mixed{
    var_dump(__METHOD__);
  }
}

function test($f) :mixed{
  call_user_func($f);
}


<<__EntryPoint>>
function main_classof() :mixed{
test(vec['T', 'T::foo']);
}
