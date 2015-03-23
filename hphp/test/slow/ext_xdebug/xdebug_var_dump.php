<?php
  class TimeStuff {
    private $timestamp;
    private $user_defined;
    private $self;
    protected $tm;
    public $date;
    function TimeStuff($ts = null)
    {
      $this->self = &$this;
      $this->timestamp = $ts === null ? time() : $ts;
      $this->user_defined = ($ts !== null);
      $this->date = date("Y-m-d H:i:s T", $this->timestamp);
      $this->tm = getdate($this->timestamp);
    }
  }
$ts1 = new TimeStuff(1092515106);
var_dump($ts1);
?>
