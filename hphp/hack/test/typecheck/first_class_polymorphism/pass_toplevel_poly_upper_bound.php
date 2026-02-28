<?hh

function poly<T as arraykey>(T $x, int $y, string $z): arraykey {
  if($x is int) {
    return $x+$y;
  } else {
    return $x.'+'.$z;
  }
}

function rcvr(
  (function(int,int,string): arraykey) $_ ,
  (function(string,int,string): arraykey) $_
) : void {}


function pass_generic(): void {
  $f = poly<>;
  rcvr($f, $f);
}
