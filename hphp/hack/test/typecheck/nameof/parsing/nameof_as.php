<?hh

class C {
  public static function f(): void {
    nameof C as string; // T171578360
  }
}
