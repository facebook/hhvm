<?hh // strict
interface Rx {}

class A {

  public function f()[rx_shallow]: void { // TODO(coeffects) abstract ctx
    // should be error
    echo 1;
  }
}
