<?hh

class C {
  public static function f(): void {
    nameof C ? true : false; // T171578360
  }
}
