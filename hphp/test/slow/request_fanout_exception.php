<?hh

<<__EntryPoint>>
function main() {
  try {
    throw new RequestFanoutLimitExceededException("Request Fanout limit exceeded");
  } catch (Exception $e) {
    echo "Caught RequestFanoutLimitExceededException, now rethrowing";
    throw $e;
  }

}
