<?hh

<<__EntryPoint>>
function main() {
  $cl = () ==> 0;

  try {
    $f = hh\dynamic_class_meth(get_class($cl), "__invoke");
    echo "ERROR\n";
    $f();
  } catch (Exception $e) {
    echo "Caught!\n";
  }

  $matches = null;
  $pat = '/^Closure\$main;([0123456789]|[12][0123456789])$/';
  if (preg_match_with_matches($pat, get_class($cl), inout $matches)) {
    $m = null;
    try {
      switch (intval($matches[1])) {
      case  0: $m = class_meth('Closure$main;0',  '__invoke'); break;
      case  1: $m = class_meth('Closure$main;1',  '__invoke'); break;
      case  2: $m = class_meth('Closure$main;2',  '__invoke'); break;
      case  3: $m = class_meth('Closure$main;3',  '__invoke'); break;
      case  4: $m = class_meth('Closure$main;4',  '__invoke'); break;
      case  5: $m = class_meth('Closure$main;5',  '__invoke'); break;
      case  6: $m = class_meth('Closure$main;6',  '__invoke'); break;
      case  7: $m = class_meth('Closure$main;7',  '__invoke'); break;
      case  8: $m = class_meth('Closure$main;8',  '__invoke'); break;
      case  9: $m = class_meth('Closure$main;9',  '__invoke'); break;
      case 10: $m = class_meth('Closure$main;10', '__invoke'); break;
      case 11: $m = class_meth('Closure$main;11', '__invoke'); break;
      case 12: $m = class_meth('Closure$main;12', '__invoke'); break;
      case 13: $m = class_meth('Closure$main;13', '__invoke'); break;
      case 14: $m = class_meth('Closure$main;14', '__invoke'); break;
      case 15: $m = class_meth('Closure$main;15', '__invoke'); break;
      case 16: $m = class_meth('Closure$main;16', '__invoke'); break;
      case 17: $m = class_meth('Closure$main;17', '__invoke'); break;
      case 18: $m = class_meth('Closure$main;18', '__invoke'); break;
      case 19: $m = class_meth('Closure$main;19', '__invoke'); break;
      case 20: $m = class_meth('Closure$main;20', '__invoke'); break;
      case 21: $m = class_meth('Closure$main;21', '__invoke'); break;
      case 22: $m = class_meth('Closure$main;22', '__invoke'); break;
      case 23: $m = class_meth('Closure$main;23', '__invoke'); break;
      case 24: $m = class_meth('Closure$main;24', '__invoke'); break;
      case 25: $m = class_meth('Closure$main;25', '__invoke'); break;
      case 26: $m = class_meth('Closure$main;26', '__invoke'); break;
      case 27: $m = class_meth('Closure$main;27', '__invoke'); break;
      case 28: $m = class_meth('Closure$main;28', '__invoke'); break;
      case 29: $m = class_meth('Closure$main;29', '__invoke'); break;
      default: echo "ERROR\n";
      }
      echo "ERROR\n";
      $m();
    } catch (Exception $e) {
      echo "Caught!\n";
    }
  } else {
    echo "ERROR\n";
  }
}
