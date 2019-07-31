<?hh // strict

function f(): void {
  $good0 = re"/\x{10FFFF}/u";
  $bad0 = re"/\x{10FFFF}/";
}
