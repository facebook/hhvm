<?hh // partial

namespace HH\Rx {

interface Exception {
  require extends \Exception;
  <<__Rx, __MaybeMutable>>
  public function getMessage(): string;
}

}
