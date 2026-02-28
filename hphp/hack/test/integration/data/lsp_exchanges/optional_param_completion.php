<?hh

function completion_area(MyFooCompletion $mfc): void {

}

class MyFooCompletion {
  public function doStuff(int $x, int $y = 0): void {}
}
