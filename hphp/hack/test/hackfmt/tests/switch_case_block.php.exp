<?hh

switch ($str) {
  case 'foo': {
    logFooEvent();
    break;
  }
  case 'bar':
    {
      $eventDetails = shape('message' => 'bar');
      logBarEvent($eventDetails);
    }
    logSomethingElse();
    break;
  default: {
    logOtherEvent();
    break;
  }
}
