<?hh

namespace HH\Client {

enum TypecheckStatus : int {
  SUCCESS = 0;
  TYPE_ERROR = 1;
  SERVER_BUSY = 2;
  COMMAND_NOT_FOUND = 3;
}

final class TypecheckResult implements \JsonSerializable {
  public function __construct(
    private TypecheckStatus $status,
    private ?string $error,
    private ?array $rawResult = null
  ) {}

  public function getStatus(): TypecheckStatus {
    return $this->status;
  }

  public function getError(): ?string {
    return $this->error;
  }

  public function triggerError(
    int $type_error_level = E_RECOVERABLE_ERROR,
    int $client_error_level = E_RECOVERABLE_ERROR,
  ): void {
    switch ($this->status) {
    case TypecheckStatus::SUCCESS:
      // No error to trigger.
      break;
    case TypecheckStatus::TYPE_ERROR:
      trigger_error($this->error, $type_error_level);
      break;
    case TypecheckStatus::SERVER_BUSY:
    case TypecheckStatus::COMMAND_NOT_FOUND:
      trigger_error($this->error, $client_error_level);
      break;
    }
  }

  public function jsonSerialize() {
    if ($this->rawResult) {
      return $this->rawResult;
    } else {
      // Return something that looks close to the hh_client response.
      return Map {
        'passed' => false,
        'errors' => Vector {
          Map {
            'message' => Vector {
              Map {
                'descr' => $this->error,
              },
            },
          },
        },
      };
    }
  }
}

function typecheck(
  string $client_name = 'hh_client',
): TypecheckResult {
  $cmd = sprintf('which %s > /dev/null 2>&1', escapeshellarg($client_name));
  $ret = null;
  $output_arr = null;
  exec($cmd, $output_arr, $ret);

  if ($ret !== 0) {
    $error_text = sprintf(
      'Hack typechecking failed: typechecker command not found: %s',
      $client_name,
    );

    return new TypecheckResult(TypecheckStatus::COMMAND_NOT_FOUND, $error_text);
  }

  $cmd = sprintf(
    '%s --timeout 0 --retries 0 --json %s 2>&1',
    escapeshellarg($client_name),
    escapeshellarg(dirname($_SERVER['SCRIPT_FILENAME'])),
  );

  $ret = null;
  $output_arr = null;
  $output = exec($cmd, $output_arr, $ret);

  // 4 -> busy
  // 6 -> just started up
  // 7 -> still starting up
  //
  // Yes this is terrible, one of these days I'll get around to fixing up the
  // hh_client return codes.
  if ($ret == 4 || $ret === 6 || $ret == 7) {
    return new TypecheckResult(
      TypecheckStatus::SERVER_BUSY,
      'Hack typechecking failed: server not ready'
    );
  }

  $json = json_decode($output, true);
  $passed = ($ret === 0) && idx($json, 'passed', false);

  if ($passed) {
    return new TypecheckResult(TypecheckStatus::SUCCESS, null, $json);
  } else {
    $errors = idx($json, 'errors');
    if ($errors) {
      $first_msg = reset(reset($errors)['message']);
      $error_text = sprintf(
        'Hack type error: %s at %s line %d',
        $first_msg['descr'],
        $first_msg['path'],
        $first_msg['line'],
      );
    } else {
      $error_text = sprintf('Hack typechecking failed: %s', $output);
    }

    return new TypecheckResult(TypecheckStatus::TYPE_ERROR, $error_text, $json);
  }
}

function typecheck_and_error(): void {
  // This is deliberately an unconfigurable convenience wrapper. If you want
  // full configurability, call typecheck() and use the TypecheckResult
  // yourself.
  typecheck()->triggerError();
}

}
