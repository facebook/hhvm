<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function good_mixed(mixed $m): void {
  switch ($m) {
    case 1: return;
  }
}

final class C {}

function good_final_class(C $c): void {
  switch ($c) {
    case new C(): return;
  }
}

function good_bool(bool $b): void {
  switch ($b) {
    case true: return;
    case false: return;
  }
}

function good_string(string $s): void {
  switch ($s)  {
    case "hi": return;
    case "hello": return;
    default: return;
  }
}

function good_arraykey(arraykey $s): void {
  switch ($s)  {
    case "hi": return;
    case 42: return;
    default: return;
  }
}

/*
function good_intersection((bool & arraykey) $s): void {
  switch ($s)  {
    case true: return;
  }
}
 */

function good_union((bool | C) $s): void {
  switch ($s)  {
    case true: return;
  }
}
