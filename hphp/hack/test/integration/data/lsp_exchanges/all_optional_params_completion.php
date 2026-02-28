<?hh

function completion_area_allopt(MyFooCompletionOptional $mfc): void {

}

class MyFooCompletionOptional {
  public function doStuff(int $x = 0, int $y = 0): void {}
}
