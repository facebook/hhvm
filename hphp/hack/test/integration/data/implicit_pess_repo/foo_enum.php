<?hh

enum MyEnum: string as string {
  A = "A";
}

class MyClass {
  public function h(): MyEnum {
    return MyEnum::A;
  }
}

function top(): void {
  $x = new MyClass();
  $y = $x->h();
}
