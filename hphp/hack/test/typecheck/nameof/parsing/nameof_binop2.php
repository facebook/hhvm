<?hh

class C {
  public static function f(): void {
    nameof C == "hello"; // T171578360
  }
}
