<?hh

function f(): void {
  $a = <<__NonRx("?", 1)>>function() {
    return 1;
  };
}
