<?hh // partial

namespace HH\Rx {

interface Exception {
  require extends \Exception;
  public function getMessage()[]: string;
}

}
