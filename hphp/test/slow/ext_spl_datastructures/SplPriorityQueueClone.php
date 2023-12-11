<?hh

function getQueueWithLittleData() :mixed{
  $q = new SplPriorityQueue();
  $q->insert('dux', 4);
  $q->insert('legati', 3);
  $q->insert('centurion', 2);
  $q->insert('munifex', 1);
  return $q;
}

function testExtractFlags() :mixed{
  $flags = vec[
    vec['SplPriorityQueue::EXTR_DATA', SplPriorityQueue::EXTR_DATA],
    vec['SplPriorityQueue::EXTR_PRIORITY', SplPriorityQueue::EXTR_PRIORITY],
    vec['SplPriorityQueue::EXTR_BOTH', SplPriorityQueue::EXTR_BOTH],
  ];
  $sources = vec[
    getQueueWithLittleData(),
    clone getQueueWithLittleData(),
  ];
  foreach ($sources as $queue) {
    foreach ($flags as $flagInfo) {
      list($name, $value) = $flagInfo;
      testExtractFlag($queue, $name, $value);
    }
  }
}

function testExtractFlag($q, $flagName, $flagValue) :mixed{
  $q->setExtractFlags($flagValue);
  echo "Get top with $flagName:\n";
  print_r($q->top());
  echo "\n";

  $q->rewind();
  $q->next();
  $q->valid();

  echo "Second in rank with $flagName:\n";
  print_r($q->current());
  echo "\n";

  echo "\n";
}


function testCloneGivesValidCopy() :mixed{
  $q = getQueueWithLittleData();
  $clonedQueue = clone $q;

  $clonedQueue->top();

  echo "Top rank (even after killing the top in a clone) is: ";
  echo $q->top() . "\n";
}

<<__EntryPoint>>
function main_spl_priority_queue_clone() :mixed{
testExtractFlags();
testCloneGivesValidCopy();
}
