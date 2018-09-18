<?hh

function f<reified T>(mixed $x) {
  try {
    $x as T;
    var_dump("yes");
  } catch (Exception $_) {
    var_dump("nope");
  }
}

f<reified int>("hello");
f<reified int>(1);
