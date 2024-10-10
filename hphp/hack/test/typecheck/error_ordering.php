//// a.php
<?hh

function g(int $x): string {
  return $x;
}

function h(integer $_): void {}

function j(?int $i, ?string $s): void {
  if ($i === $s) {
  }
  $i[] = 1;
}

//// b.php
<?hh

function g2(int $x): string {
  return $x;
}

function h2(integer $_): void {}

function j2(?int $i, ?string $s): void {
  if ($i === $s) {
  }
  $i[] = 1;
}
