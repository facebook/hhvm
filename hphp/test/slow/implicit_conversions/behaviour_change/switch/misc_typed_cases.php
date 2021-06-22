<?hh

<<__EntryPoint>>
function main() {
  f(true);
  f(false);
  f(null);
  f("str");
  f(1234);
  f(1234.5);
  f('1234');
  f('1234.5');
  f(42);
  f(42.5);
  f('42');
  f('42.5');
  f("");
  f(new Derp());
  f(new Herp('1'));
  f(new Herp(1));
  f(new stdClass());
  f(1);
  f(0);
  f("0");
  f(STDIN);
  f(vec[1]);
  f(vec['1']);
  f(Vector{1});
  f(Vector{'1'});
}

class Derp {}

class Herp {
  public function __construct(public $x) {}
}

function f($arg) {
  var_dump($arg);
  print "goes to:\n";
  switch ($arg) {
    case true:
      echo "true";
      break;
    case false:
      echo "false";
      break;
    case null:
      echo "null";
      break;
    case "str":
      echo "str";
      break;
    case "1234":
      echo "'1234'";
      break;
    case "1234.5":
      echo "'1234.5'";
      break;
    case 42:
      echo "42";
      break;
    case 42.5:
      echo "42.5";
      break;
    case STDIN:
      echo "stdin";
      break;
    case STDOUT:
      echo "stdout";
      break;
    case new Derp():
      echo "derp";
      break;
    case new Herp(1):
      echo "Herp(1)";
      break;
    case vec[1]:
      echo "vec[1]";
      break;
    case Vector{1}:
      echo "Vector{1}";
      break;
    default:
      echo "default";
      break;
  }
  echo "\n";
}
