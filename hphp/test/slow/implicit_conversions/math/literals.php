<?hh

class Foo {}

function handler($_, $str, $file, $line) {
  if (preg_match('/Object of class (.*) could not be converted to (float|int)/', $str)) {
    throw new TypecastException($str." in $file on line $line");
  }
  return false;
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(handler<>);
  plus();
  minus();
  mul();
  div();
  mod();
  pow_();
}

function plus(): void {
  echo 'plus<';
  echo 0 + -10;
  echo 1.234 + INF;
  echo NAN + true;
  echo false + null;
  echo STDIN + "string";
  try {
    echo new Foo() + -INF;
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }

  echo ">\n";

  $i = 1;
  echo "pluseq<";
  $i += 0;
  $i += -10;
  $i += 1.234;
  $i += INF;
  $i += NAN;
  $i += true;
  $i += false;
  $i += STDIN;
  $i += "string";
  try {
    $i += new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo ">\n";
}

function minus(): void {
  echo 'minus<';
  echo 0 - -10;
  echo 1.234 - INF;
  echo NAN - true;
  echo false - null;
  echo STDIN - "string";
  try {
    echo new Foo() - -INF;
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo ">\n";

  $i = 1;
  echo "minuseq<";
  $i -= 0;
  $i -= -10;
  $i -= 1.234;
  $i -= INF;
  $i -= NAN;
  $i -= true;
  $i -= false;
  $i -= STDIN;
  $i -= "string";
  try {
    $i -= new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo ">\n";
}

function mul(): void {
  echo 'mul<';
  echo 0 * -10;
  echo 1.234 * INF;
  echo NAN * true;
  echo false * null;
  echo STDIN * "string";
  try {
    echo new Foo() * -INF;
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo ">\n";

  $i = 1;
  echo "muleq<";
  $i *= 0;
  $i *= -10;
  $i *= 1.234;
  $i *= INF;
  $i *= NAN;
  $i *= true;
  $i *= false;
  $i *= STDIN;
  $i *= "string";
  try {
    $i *= new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo ">\n";
}

function div(): void {
  echo 'div<';
  echo 0 / -10;
  echo 1.234 / INF;
  echo null / true;
  echo false / NAN;
  echo "string" / STDIN;
  try {
    echo -INF / new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo ">\n";

  $i = 1;
  echo "diveq<";
  $i /= -10;
  $i /= 1.234;
  $i /= INF;
  $i /= NAN;
  $i /= true;
  $i /= STDIN;
  $i /= "12string";
  try {
    $i /= new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo "\n";
}

function mod(): void {
  echo 'mod<';
  echo 0 % -10;
  echo null % true;
  echo false % NAN;
  echo STDIN % "12string";
  echo varray[42] % dict['foobar' => false];
  try {
    echo -INF % new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo ">\n";

  $i = 1;
  echo "modeq<";
  $i %= -10;
  $i %= 1.234;
  $i %= NAN;
  $i %= true;
  $i %= STDIN;
  $i %= "12string";
  $i %= varray[42];
  $i %= dict['foobar' => false];
  try {
    $i %= new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo ">\n";
}

function pow_(): void {
  echo 'pow<';
  echo 0 ** -10;
  echo 1.234 ** INF;
  echo NAN ** true;
  echo false ** null;
  echo STDIN ** "string";
  echo varray[42] ** dict['foobar' => false];
  try {
    echo new Foo() ** -INF;
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo ">\n";

  $i = 1;
  echo "poweq<";
  $i  **=  0;
  $i  **=  -10;
  $i  **=  1.234;
  $i  **=  INF;
  $i  **=  NAN;
  $i  **=  true;
  $i  **=  false;
  $i  **=  STDIN;
  $i  **=  "string";
  $i  **=  varray[42];
  $i  **=  dict['foobar' => false];
  try {
    $i **= new Foo();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
  echo $i;
  echo ">\n";
}
