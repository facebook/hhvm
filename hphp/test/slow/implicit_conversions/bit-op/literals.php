<?hh

class Foo {}

<<__EntryPoint>>
function main(): void {
  not();
  and();
  or();
  xor();
  shl();
  shr();
}

function not(): void {
  echo 'not<';
  echo ~1.234;
  echo ">\n";
}

function and(): void {
  echo 'and<';
  echo 0 & -10;
  echo 1.234 & INF;
  echo NAN & true;
  echo false & null;
  echo STDIN & "string";
  echo "string" & "string";
  echo varray[42] & dict['foobar' => false];
  echo ">\n";

  $i = 1;
  echo "andeq<";
  $i &= 0;
  $i &= -10;
  $i &= 1.234;
  $i &= INF;
  $i &= NAN;
  $i &= true;
  $i &= false;
  $i &= STDIN;
  $i &= "string";
  $i &= varray[42];
  $i &= dict['foobar' => false];
  echo $i;

  $i = "string";
  $i &= "string";
  echo "$i>\n";
}

function or(): void {
  echo 'or<';
  echo 0 | -10;
  echo 1.234 | INF;
  echo NAN | true;
  echo false | null;
  echo STDIN | "string";
  echo "string" | "string";
  echo varray[42] | dict['foobar' => false];
  echo ">\n";

  $i = 1;
  echo "oreq<";
  $i |= 0;
  $i |= -10;
  $i |= 1.234;
  $i |= INF;
  $i |= NAN;
  $i |= true;
  $i |= false;
  $i |= STDIN;
  $i |= "string";
  $i |= varray[42];
  $i |= dict['foobar' => false];
  echo $i;

  $i = "string";
  $i |= "string";
  echo "$i>\n";
}

function xor(): void {
  echo 'xor<';
  echo 0 ^ -10;
  echo 1.234 ^ INF;
  echo NAN ^ true;
  echo false ^ null;
  echo STDIN ^ "string";
  echo "string" ^ "string";
  echo varray[42] ^ dict['foobar' => false];
  echo ">\n";

  $i = 1;
  echo "xoreq<";
  $i ^= 0;
  $i ^= -10;
  $i ^= 1.234;
  $i ^= INF;
  $i ^= NAN;
  $i ^= true;
  $i ^= false;
  $i ^= STDIN;
  $i ^= "string";
  $i ^= varray[42];
  $i ^= dict['foobar' => false];
  echo $i;

  $i = "string";
  $i ^= "string";
  echo "$i>\n";
}

function shl(): void {
  echo 'shl<';
  echo 0 << -10;
  echo 1.234 << INF;
  echo NAN << true;
  echo false << null;
  echo STDIN << "string";
  echo "string" << "string";
  echo varray[42] << dict['foobar' => false];
  echo ">\n";

  $i = 1;
  echo "shleq<";
  $i <<= 0;
  $i <<= -10;
  $i <<= 1.234;
  $i <<= INF;
  $i <<= NAN;
  $i <<= true;
  $i <<= false;
  $i <<= STDIN;
  $i <<= "string";
  $i <<= varray[42];
  $i <<= dict['foobar' => false];
  echo $i;

  $i = "string";
  $i <<= "string";
  echo "$i>\n";
}

function shr(): void {
  echo 'shr<';
  echo 0 >> -10;
  echo 1.234 >> INF;
  echo NAN >> true;
  echo false >> null;
  echo STDIN >> "string";
  echo "string" >> "string";
  echo varray[42] >> dict['foobar' => false];
  echo ">\n";

  $i = 1;
  echo "shreq<";
  $i >>= 0;
  $i >>= -10;
  $i >>= 1.234;
  $i >>= INF;
  $i >>= NAN;
  $i >>= true;
  $i >>= false;
  $i >>= STDIN;
  $i >>= "string";
  $i >>= varray[42];
  $i >>= dict['foobar' => false];
  echo $i;

  $i = "string";
  $i >>= "string";
  echo "$i>\n";
}
