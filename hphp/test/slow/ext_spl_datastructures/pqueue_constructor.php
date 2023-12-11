<?hh

class MyPriorityQueue extends \SplPriorityQueue {
  public function __construct() {
    $this->setExtractFlags(self::EXTR_DATA);
  }

  public function sort() :mixed{
    $sortedList = vec[];

    while ($this->valid()) {
      $sortedList[] = $this->extract();
    }

    return array_reverse($sortedList);
  }
}


<<__EntryPoint>>
function main_pqueue_constructor() :mixed{
$data  = vec[
  vec['test1', 1],
  vec['test3', 3],
  vec['test2', 2],
];
$queue = new MyPriorityQueue();

foreach ($data as $entry) {
  $queue->insert($entry[0], $entry[1]);
}

var_dump($queue->sort());
}
