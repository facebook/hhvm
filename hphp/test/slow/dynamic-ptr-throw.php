<?hh

function no_dyn() :mixed{ echo __FUNCTION__."\n"; }
<<__DynamicallyCallable>> function dyn() :mixed{ echo __FUNCTION__."\n"; }
<<__DynamicallyCallable>> function reified_fun<reify T>() :mixed{
  echo __FUNCTION__."\n";
}

class Cls {
  static function static_no_dyn() :mixed{ echo __METHOD__."\n"; }
  function inst_no_dyn() :mixed{ echo __METHOD__."\n"; }
  <<__DynamicallyCallable>> static function static_dyn() :mixed{echo __METHOD__."\n";}
  <<__DynamicallyCallable>> function inst_dyn() :mixed{ echo __METHOD__."\n"; }
  <<__DynamicallyCallable>> static function reified_dyn<reify T>() :mixed{
    echo __METHOD__."\n";
  }
}

function test_it($fname, $cname = null) :mixed{
  try {
    if ($cname) $f = HH\dynamic_class_meth($cname, $fname);
    else        $f = HH\dynamic_fun($fname);
  } catch (InvalidArgumentException $ex) {
    echo "Caught: ".$ex->getMessage()."\n";
    return;
  }
  $f();
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(($_n, $str) ==> { echo "Warning: $str\n"; return true; });

  test_it('dyn');
  test_it('no_dyn');
  test_it('static_dyn', 'Cls');
  test_it('static_no_dyn', 'Cls');

  test_it('inst_dyn', 'Cls');
  test_it('inst_no_dyn', 'Cls');

  test_it('bad_func_name');
  test_it('bad_meth_name', 'Cls');
  test_it('static_dyn', 'BadClassName');

  test_it('reified_fun');
  test_it('reified_dyn', 'Cls');
}
