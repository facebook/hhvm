<?hh

function no_dyn() { echo __FUNCTION__."\n"; }
<<__DynamicallyCallable>> function dyn() { echo __FUNCTION__."\n"; }

class Cls {
  static function static_no_dyn() { echo __METHOD__."\n"; }
  function inst_no_dyn() { echo __METHOD__."\n"; }
  <<__DynamicallyCallable>> static function static_dyn() {echo __METHOD__."\n";}
  <<__DynamicallyCallable>> function inst_dyn() { echo __METHOD__."\n"; }
}

function test_it($fname, $cname = null) {
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
function main() {
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
}
