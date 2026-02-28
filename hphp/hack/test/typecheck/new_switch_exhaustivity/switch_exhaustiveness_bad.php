<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class C {}

function bad_class(C $c): void {
  switch ($c) {
    case new C(): return;
  }
}

function bad_int(int $i): void {
  switch ($i) {
    case 42: return;
    case 24: return;
  }
}

function bad_string(string $s): void {
  switch ($s)  {
    case "hi": return;
    case "hello": return;
  }
}

function bad_arraykey(arraykey $s): void {
  switch ($s)  {
    case "hi": return;
    case 42: return;
  }
}

interface I {}
interface J {}

function bad_intersection((I & J) $o1, (I & J) $o2): void {
  switch ($o1)  {
    case $o2: return;
  }
}

function bad_union((bool | string) $s): void {
  switch ($s)  {
    case true: return;
  }
}
