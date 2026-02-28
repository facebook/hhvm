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

function with_exn($fn): mixed {
  try {
    return $fn();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
    return null;
  }
}

function not(): void {
  echo 'not<';
  echo with_exn(() ==> ~1.234);
  echo ">\n";
}

function and(): void {
  echo 'and<';
  echo with_exn(() ==> 0 & -10);
  echo with_exn(() ==> 1.234 & INF);
  echo with_exn(() ==> NAN & true);
  echo with_exn(() ==> false & null);
  echo with_exn(() ==> HH\stdin() & "string");
  echo with_exn(() ==> "string" & "string");
  echo with_exn(() ==> vec[42] & dict['foobar' => false]);
  echo ">\n";

  $i = 1;
  echo "andeq<";
  $i = with_exn(() ==> { $i &= 0; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= -10; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= 1.234; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= INF; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= NAN; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= true; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= false; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= HH\stdin(); return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= "string"; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= vec[42]; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i &= dict['foobar' => false]; return $i; }) ?? $i;
  echo $i;

  $i = "string";
  $i = with_exn(() ==> { $i &= "string"; return $i; }) ?? $i;
  echo "$i>\n";
}

function or(): void {
  echo 'or<';
  echo with_exn(() ==> 0 | -10);
  echo with_exn(() ==> 1.234 | INF);
  echo with_exn(() ==> NAN | true);
  echo with_exn(() ==> false | null);
  echo with_exn(() ==> HH\stdin() | "string");
  echo with_exn(() ==> "string" | "string");
  echo with_exn(() ==> vec[42] | dict['foobar' => false]);
  echo ">\n";

  $i = 1;
  echo "oreq<";
  $i = with_exn(() ==> { $i |= 0; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= -10; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= 1.234; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= INF; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= NAN; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= true; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= false; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= HH\stdin(); return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= "string"; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= vec[42]; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i |= dict['foobar' => false]; return $i; }) ?? $i;
  echo $i;

  $i = "string";
  $i = with_exn(() ==> { $i |= "string"; return $i; }) ?? $i;
  echo "$i>\n";
}

function xor(): void {
  echo 'xor<';
  echo with_exn(() ==> 0 ^ -10);
  echo with_exn(() ==> 1.234 ^ INF);
  echo with_exn(() ==> NAN ^ true);
  echo with_exn(() ==> false ^ null);
  echo with_exn(() ==> HH\stdin() ^ "string");
  echo with_exn(() ==> "string" ^ "string");
  echo with_exn(() ==> vec[42] ^ dict['foobar' => false]);
  echo ">\n";

  $i = 1;
  echo "xoreq<";
  $i = with_exn(() ==> { $i ^= 0; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= -10; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= 1.234; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= INF; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= NAN; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= true; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= false; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= HH\stdin(); return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= "string"; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= vec[42]; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i ^= dict['foobar' => false]; return $i; }) ?? $i;
  echo $i;

  $i = "string";
  $i = with_exn(() ==> { $i ^= "string"; return $i; }) ?? $i;
  echo "$i>\n";
}

function shl(): void {
  echo 'shl<';
  echo with_exn(() ==> 0 << -10);
  echo with_exn(() ==> 1.234 << INF);
  echo with_exn(() ==> NAN << true);
  echo with_exn(() ==> false << null);
  echo with_exn(() ==> HH\stdin() << "string");
  echo with_exn(() ==> "string" << "string");
  echo with_exn(() ==> vec[42] << dict['foobar' => false]);
  echo ">\n";

  $i = 1;
  echo "shleq<";
  $i = with_exn(() ==> { $i <<= 0; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= -10; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= 1.234; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= INF; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= NAN; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= true; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= false; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= HH\stdin(); return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= "string"; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= vec[42]; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i <<= dict['foobar' => false]; return $i; }) ?? $i;
  echo $i;

  $i = "string";
  $i = with_exn(() ==> { $i <<= "string"; return $i; }) ?? $i;
  echo "$i>\n";
}

function shr(): void {
  echo 'shr<';
  echo with_exn(() ==> 0 >> -10);
  echo with_exn(() ==> 1.234 >> INF);
  echo with_exn(() ==> NAN >> true);
  echo with_exn(() ==> false >> null);
  echo with_exn(() ==> HH\stdin() >> "string");
  echo with_exn(() ==> "string" >> "string");
  echo with_exn(() ==> vec[42] >> dict['foobar' => false]);
  echo ">\n";

  $i = 1;
  echo "shreq<";
  $i = with_exn(() ==> { $i >>= 0; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= -10; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= 1.234; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= INF; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= NAN; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= true; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= false; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= HH\stdin(); return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= "string"; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= vec[42]; return $i; }) ?? $i;
  $i = with_exn(() ==> { $i >>= dict['foobar' => false]; return $i; }) ?? $i;
  echo $i;

  $i = "string";
  $i = with_exn(() ==> { $i >>= "string"; return $i; }) ?? $i;
  echo "$i>\n";
}
