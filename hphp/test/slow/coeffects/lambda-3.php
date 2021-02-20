<?hh

function rx_local($f) {
  $f();
}

function pure($f)[] {
  $f();
};

class C {
  static function defaults() {
    $f = ()[rx] ==> { echo "in lambda\n"; };
    pure($f);
  }
}

<<__EntryPoint>>
function main() {
  $f1 = () ==> { echo "in lambda\n"; };
  $f2 = ()[] ==> { echo "in lambda\n"; };
  $f3 = ()[rx] ==> { echo "in lambda\n"; };
  $f4 = ()[rx_local] ==> { echo "in lambda\n"; };
  pure($f1);
  pure($f2);
  pure($f3);
  pure($f4);
  rx_local($f1);
  rx_local($f2);
  rx_local($f3);
  rx_local($f4);

  C::defaults();
}
