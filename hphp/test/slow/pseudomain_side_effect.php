<?hh
function foo() {
  echo "haha";
}

class Foo {
  const NAME = 'test function';
}

trait ezcReflectionReturnInfo {
    function getReturnType() { /*1*/ }
    function getReturnDescription() { /*2*/ }
}

interface iTemplate
{
    public function setVariable($name, $var);
    public function getHtml($template);
}

type age = int;

newtype Point = (int, int);

enum Size: int {
  SMALL = 0;
  MEDIUM = 1;
  LARGE = 2;
  X_LARGE = 3;
}

const t_int = 0;
const t_str = "";
const t_dbl = 0.0;
const t_true = True;
const t_false = False;

const a = t_int;
const b = Foo::NAME;
const c = Size::LARGE;
