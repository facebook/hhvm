<?hh

class Foo {}

<<__EntryPoint>>
function main(): void {
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
  echo new Foo() + -INF;
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
  $i += new Foo();
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
  echo new Foo() - -INF;
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
  $i -= new Foo();
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
  echo new Foo() * -INF;
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
  $i *= new Foo();
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
  echo -INF / new Foo();
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
  $i /= new Foo();
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
  echo -INF % new Foo();
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
  $i %= new Foo();
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
  echo new Foo() ** -INF;
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
  $i  **=  new Foo();
  echo $i;
  echo ">\n";
}
