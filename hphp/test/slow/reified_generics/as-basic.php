<?hh

function f<reify T>(mixed $x) {
  try {
    $x as T;
    var_dump("yes");
  } catch (Exception $_) {
    var_dump("nope");
  }
}

f<reify int>("hello");
f<reify int>(1);
