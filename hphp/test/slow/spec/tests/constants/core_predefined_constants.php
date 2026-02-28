<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function trace($text, $pdc)
:mixed{
    echo "$text: ";
    var_dump($pdc);
}

function ComputeResult()
:mixed{
    echo "Inside ComputeResult\n";
    trace("__FUNCTION__", __FUNCTION__);
    trace("__METHOD__", __METHOD__);
    trace("__CLASS__", __CLASS__);
    trace("__TRAIT__", __TRAIT__);
    trace("__NAMESPACE__", __NAMESPACE__);
}

class Date
{
    public function __construct()
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__CLASS__", __CLASS__);
        trace("__FUNCTION__", __FUNCTION__);
        trace("__TRAIT__", __TRAIT__);
        trace("__NAMESPACE__", __NAMESPACE__);

        // ...
    }

    public function setDay($day)
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
        trace("__LINE__", __LINE__);

        $this->priv1();
        $this::spf1();
    }

// public vs. private doesn't matter

    private function priv1()
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }

    static public function spf1()
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }

}

class DatePlus extends Date
{
    public function xx()
:mixed    {
        trace("__CLASS__", __CLASS__);
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }
}
<<__EntryPoint>>
function entrypoint_core_predefined_constants(): void {
  error_reporting(-1);

  trace("__LINE__", __LINE__);

  trace("__FILE__", __FILE__);

  trace("__DIR__", __DIR__);
  var_dump(dirname(__FILE__));

  trace("__LINE__", __LINE__);

  trace("__NAMESPACE__", __NAMESPACE__);

  echo "-----------------------------------------\n";

  echo "At the top level of a script\n";
  trace("__FUNCTION__", __FUNCTION__);

  echo "-----------------------------------------\n";

  echo "At the top level of a script and outside all classes\n";
  trace("__METHOD__", __METHOD__);

  echo "-----------------------------------------\n";

  echo "Outside all classes\n";
  trace("__CLASS__", __CLASS__);

  echo "-----------------------------------------\n";

  echo "Outside all classes\n";
  trace("__TRAIT__", __TRAIT__);

  echo "-----------------------------------------\n";

  ComputeResult();

  echo "-----------------------------------------\n";

  $date1 = new Date;
  $date1->setDay(22);

  echo "-----------------------------------------\n";

  $datePlus1 = new DatePlus;
  $datePlus1->xx();

  include_once('includefile.inc');
  include_file();
}
