<?hh
class SplMinHeap2 extends SplMinHeap {
    public function compare($a, $b) :mixed{
        return -parent::compare($a,$b);
    }
}

class SplMaxHeap2 extends SplMaxHeap {
    public function compare($a, $b) :mixed{
        return -parent::compare($a,$b);
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $h = new SplMinHeap2();
  $h->insert(1);
  $h->insert(6);
  $h->insert(5);
  $h->insert(2);
  var_dump($h->top());
  $h = new SplMaxHeap2();
  $h->insert(1);
  $h->insert(6);
  $h->insert(5);
  $h->insert(2);
  var_dump($h->top());
}
