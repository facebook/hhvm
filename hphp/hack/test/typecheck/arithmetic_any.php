////file1.php
<?hh

  function returnsAny() { return 5; }

////file2.php
<?hh

  // ADDITION
function doAddIntAny(int $x): void {
  $y = $x + returnsAny();
  hh_show($y);
}

function doAddAnyInt(int $x): void {
  $y = returnsAny() + $x;
  hh_show($y);
}

function doAddFloatAny(float $x): void {
  $y = $x + returnsAny();
  hh_show($y);
}

function doAddAnyFloat(float $x): void {
  $y = returnsAny() + $x;
  hh_show($y);
}

function doAddAnyAny(): void {
  $y = returnsAny() + returnsAny();
  hh_show($y);
}

function doAddNumAny(num $x): void {
  $y = $x + returnsAny();
  hh_show($y);
}

function doAddAnyNum(num $x): void {
  $y = returnsAny() + $x;
  hh_show($y);
}

  // DIVISION
function doDivAnyFloat(float $x): void {
  $y = returnsAny() / 2.0;
  hh_show($y);
}

function doDivFloatAny(float $x): void {
  $y = 2.0 / returnsAny();
  hh_show($y);
}

function doDivAnyAny(): void {
  $y = returnsAny() / returnsAny();
  hh_show($y);
}


function doDivAnyInt(int $x): void {
  $y = returnsAny() / $x;
  hh_show($y);
}

function doDivIntAny(int $x): void {
  $y = $x / returnsAny();
  hh_show($y);
}

function sorted<T>(
  Traversable<T> $collection,
  ?(function(T, T): int) $comparator = null,
): Vector<T> {
  return Vector {};
}

function TestSort():void {
  $v = sorted(varray[],
              function ($x, $y) { hh_show($x); hh_show($y); return $x - $y; });
}

function TestUnion(?int $x, bool $b):void {
  if ($b) {
    $x = 3;
  }
  /* HH_FIXME[4110] */
  $y = $x - returnsAny();
  hh_show($y);
}

function doMulIntFloat(int $x, float $y): void {
  $z = $x * $y;
  hh_show($z);
}
