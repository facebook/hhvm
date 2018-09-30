<?hh

trait T {
  <<__LateInit>> public property $p;
};

class C {
  use T;
}

echo "OK\n";
