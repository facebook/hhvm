<?hh // partial

namespace HH\Rx {

interface Exception {
  require extends \Exception;
  <<__Pure, __MaybeMutable>>
  public function getMessage(): string;
}

}
