<?hh

function s(string $s): void {}

class C {
  public static function f(): void {
    nameof C |> s($$);
  }
}
