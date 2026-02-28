<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface GIComparable<T> {
  public function Compare(T $x): int;
}
interface IComparable {
  public function Compare(mixed $x): int;
}

class MinMax<T as GIComparable<T>> {
  private T $min;
  private T $max;
  public function __construct(T $x) {
    $this->min = $x;
    $this->max = $x;
  }
  public function Add(T $x): void {
    if ($x->Compare($this->min) < 0) {
      $this->min = $x;
    }
    if ($x->Compare($this->max) > 0) {
      $this->max = $x;
    }
  }
  public function GetMin(): T {
    return $this->min;
  }
  public function GetMax(): T {
    return $this->max;
  }
}
;

class MyInt implements GIComparable<MyInt> {
  public function __construct(public int $item) {}
  public function Compare(MyInt $mi): int {
    return $this->item - $mi->item;
  }
}

// Lexicographic ordering
class MyPair<Tx as GIComparable<Tx>, Ty as GIComparable<Ty>>
  implements GIComparable<MyPair<Tx, Ty>> {
  public function __construct(public Tx $fst, public Ty $snd) {}
  public function Compare(MyPair<Tx, Ty> $that): int {
    $c = $this->fst->Compare($that->fst);
    if ($c !== 0)
      return $c;
    return $this->snd->Compare($that->snd);
  }
}

function Test(): void {
  $two = new MyInt(2);
  $three = new MyInt(3);
  $four = new MyInt(4);
  $mm = new MinMax($three);
  $mm->Add($two);
  $mm->Add($four);
  $mm->Add($three);
  echo 'min is ', $mm->GetMin()->item;
  echo ' and max is ', $mm->GetMax()->item, "\n";

  $twotwo = new MyPair($two, $two);
  $twothree = new MyPair($two, $three);
  $fourtwo = new MyPair($four, $two);
  $mm = new MinMax($twotwo);
  $mm->Add($twothree);
  $mm->Add($fourtwo);
  echo 'min is (', $mm->GetMin()->fst->item, ',', $mm->GetMin()->snd->item, ')';
  echo ' and max is (', $mm->GetMax()->fst->item, ',', $mm->GetMax()->snd->item, ")\n";
}

// Test();
