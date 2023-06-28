<?hh

class MyPriorityQueue extends \SplPriorityQueue {
  public function __construct() {
    $this->setExtractFlags(self::EXTR_DATA);
  }

  public function sort() :mixed{
    $sortedList = varray[];

    while ($this->valid()) {
      $sortedList[] = $this->extract();
    }

    return array_reverse($sortedList);
  }
}


<<__EntryPoint>>
function main_pqueue_constructor() :mixed{
$data  = varray[
  varray['test1', 1],
  varray['test3', 3],
  varray['test2', 2],
];
$queue = new MyPriorityQueue();

foreach ($data as $entry) {
  $queue->insert($entry[0], $entry[1]);
}

var_dump($queue->sort());
}
