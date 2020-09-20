<?hh

class C {
public function foo(): void {
  f(() ==> {
    // hackfmt-ignore - this should be left as-is
    vec[
      100, 200, 300,
      400, 500, 600,
      700, 800, 900,
      100, 200, 300,
      400, 500, 600,
      700, 800, 900,
      100, 200, 300,
      400, 500, 600,
      700, 800, 900,
    ];
  });
}
}
