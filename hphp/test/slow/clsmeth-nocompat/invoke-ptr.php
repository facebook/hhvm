<?hh

<<__EntryPoint>>
function main() {
  $cl = () ==> 0;

  try {
    $f = hh\dynamic_class_meth(get_class($cl), "__invoke");
    echo "ERROR: shouldn't be able to call dynamically\n";
    $f();
  } catch (Exception $e) {
    echo "Caught!\n";
  }

  $matches = null;
  // This will start failing if we get more closures in systemlib and
  // extensions; change the pattern and extend the switch if needed
  $pat = '/^Closure\$main;[123456789][0123456789]{0,2}$/';
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
      case 30: $m = class_meth('Closure$main;30', '__invoke'); break;
      case 31: $m = class_meth('Closure$main;31', '__invoke'); break;
      case 32: $m = class_meth('Closure$main;32', '__invoke'); break;
      case 33: $m = class_meth('Closure$main;33', '__invoke'); break;
      case 34: $m = class_meth('Closure$main;34', '__invoke'); break;
      case 35: $m = class_meth('Closure$main;35', '__invoke'); break;
      case 36: $m = class_meth('Closure$main;36', '__invoke'); break;
      case 37: $m = class_meth('Closure$main;37', '__invoke'); break;
      case 38: $m = class_meth('Closure$main;38', '__invoke'); break;
      case 39: $m = class_meth('Closure$main;39', '__invoke'); break;
      case 40: $m = class_meth('Closure$main;40', '__invoke'); break;
      case 41: $m = class_meth('Closure$main;41', '__invoke'); break;
      case 42: $m = class_meth('Closure$main;42', '__invoke'); break;
      case 43: $m = class_meth('Closure$main;43', '__invoke'); break;
      case 44: $m = class_meth('Closure$main;44', '__invoke'); break;
      case 45: $m = class_meth('Closure$main;45', '__invoke'); break;
      case 46: $m = class_meth('Closure$main;46', '__invoke'); break;
      case 47: $m = class_meth('Closure$main;47', '__invoke'); break;
      case 48: $m = class_meth('Closure$main;48', '__invoke'); break;
      case 49: $m = class_meth('Closure$main;49', '__invoke'); break;
      case 50: $m = class_meth('Closure$main;50', '__invoke'); break;
      case 51: $m = class_meth('Closure$main;51', '__invoke'); break;
      case 52: $m = class_meth('Closure$main;52', '__invoke'); break;
      case 53: $m = class_meth('Closure$main;53', '__invoke'); break;
      case 54: $m = class_meth('Closure$main;54', '__invoke'); break;
      case 55: $m = class_meth('Closure$main;55', '__invoke'); break;
      case 56: $m = class_meth('Closure$main;56', '__invoke'); break;
      case 57: $m = class_meth('Closure$main;57', '__invoke'); break;
      case 58: $m = class_meth('Closure$main;58', '__invoke'); break;
      case 59: $m = class_meth('Closure$main;59', '__invoke'); break;
      case 60: $m = class_meth('Closure$main;60', '__invoke'); break;
      case 61: $m = class_meth('Closure$main;61', '__invoke'); break;
      case 62: $m = class_meth('Closure$main;62', '__invoke'); break;
      case 63: $m = class_meth('Closure$main;63', '__invoke'); break;
      case 64: $m = class_meth('Closure$main;64', '__invoke'); break;
      case 65: $m = class_meth('Closure$main;65', '__invoke'); break;
      case 66: $m = class_meth('Closure$main;66', '__invoke'); break;
      case 67: $m = class_meth('Closure$main;67', '__invoke'); break;
      case 68: $m = class_meth('Closure$main;68', '__invoke'); break;
      case 69: $m = class_meth('Closure$main;69', '__invoke'); break;
      default: echo "ERROR: extend switch statement\n";
      }
      echo "ERROR\n";
      $m();
    } catch (Exception $e) {
      echo "Caught!\n";
    }
  } else {
    echo "ERROR: extend pattern\n".get_class($cl);
  }
}
