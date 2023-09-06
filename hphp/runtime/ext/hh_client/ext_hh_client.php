<?hh // partial

namespace __SystemLib\HH\Client {

use \HH\Client\TypecheckStatus;
use \HH\Client\TypecheckResult;

abstract final class CacheKeys {
  const string TIME_CACHE_KEY = '__systemlib__hh_client_time';
  const string RESULT_CACHE_KEY = '__systemlib__hh_client_result';
}

function locate_hh(string $client_name): ?string {
  if (\strlen($client_name) > 0 && $client_name[0] == '/') {
    if (\is_executable($client_name)) {
      return $client_name;
    } else {
      return null;
    }
  }

  $base = \dirname(\HH\__internal\hhvm_binary());

  $next_to_hhvm = $base . '/' . $client_name;
  if (\is_executable($next_to_hhvm)) {
    return $next_to_hhvm;
  }

  $oss_build = $base . '/../hack/bin/' . $client_name;
  if (\is_executable($oss_build)) {
    return $oss_build;
  }

  $cmd = \sprintf('which %s > /dev/null 2>&1', \escapeshellarg($client_name));
  $ret = null;
  $output_arr = null;
  \exec($cmd, inout $output_arr, inout $ret);

  if ($ret === 0) {
    return $client_name;
  } else {
    return null;
  }
}

function typecheck_impl(string $input_client_name): TypecheckResult {
  $client_name = locate_hh($input_client_name);

  if (!$client_name) {
    $error_text = \sprintf(
      'Hack typechecking failed: typechecker command not found: %s',
      $input_client_name,
    );

    return new TypecheckResult(TypecheckStatus::COMMAND_NOT_FOUND, $error_text);
  }

  // Use a largeish timeout in CLI mode, so we can wait for hh_server startup if
  // we need to. CLI is both tolerant of occasionally larger latencies, and also
  // we want to make sure that we don't *always* get an error that the server
  // isn't ready on the first CLI script execution in a repo.
  $timeout = \php_sapi_name() === 'cli' ? 5 : 0;

  $cmd = \sprintf(
    '%s --timeout %d --retries 0 --json %s 2>&1',
    \escapeshellarg($client_name),
    $timeout,
    \escapeshellarg(\dirname(\HH\global_get('_SERVER')['SCRIPT_FILENAME'])),
  );

  $ret = null;
  $output_arr = null;
  $output = \exec($cmd, inout $output_arr, inout $ret);

  // 7 -> timeout, or ran out of retries
  if ($ret == 7) {
    return new TypecheckResult(
      TypecheckStatus::SERVER_BUSY,
      'Hack typechecking failed: server not ready. Unless you have run '.
      '"hh_client" manually, you may be missing Hack type errors! Once the '.
      'typechecker server has started up, the errors, if any, will then show '.
      'up in this error log.'
    );
  }

  $json = \json_decode($output, true);

  if (!$json) {
    // This is typically "could not find hhconfig", which of course uses the
    // same exit code as "type error". See above about fixing this.
    return new TypecheckResult(
      TypecheckStatus::OTHER_ERROR,
      \implode(' ', $output_arr)
    );
  }

  $passed = ($ret === 0) && \hphp_array_idx($json, 'passed', false);

  if ($passed) {
    return new TypecheckResult(TypecheckStatus::SUCCESS, null, $json);
  } else {
    $errors = \hphp_array_idx($json, 'errors', null);
    if ($errors) {
      $first_msg = $errors[0]['message'][0];
      $error_text = \sprintf(
        'Hack type error: %s at %s line %d',
        $first_msg['descr'],
        $first_msg['path'],
        $first_msg['line'],
      );
    } else {
      $error_text = \sprintf('Hack typechecking failed: %s', $output);
    }

    return new TypecheckResult(TypecheckStatus::TYPE_ERROR, $error_text, $json);
  }
}

}

namespace HH\Client {

use \__SystemLib\HH\Client\CacheKeys;

enum TypecheckStatus : int {
  SUCCESS = 0;
  TYPE_ERROR = 1;
  SERVER_BUSY = 2;
  COMMAND_NOT_FOUND = 3;

  OTHER_ERROR = -1;
}

final class TypecheckResult implements \JsonSerializable {
  public function __construct(
    private TypecheckStatus $status,
    private ?string $error,
    private ?varray_or_darray $rawResult = null
  ) {}

  public function getStatus(): TypecheckStatus {
    return $this->status;
  }

  public function getError(): ?string {
    return $this->error;
  }

  public function triggerError(
    int $type_error_level = \E_RECOVERABLE_ERROR,
    int $client_error_level = \E_RECOVERABLE_ERROR,
    int $server_busy_level = \E_WARNING,
  ): void {
    switch ($this->status) {
    case TypecheckStatus::SUCCESS:
      // No error to trigger.
      break;
    case TypecheckStatus::TYPE_ERROR:
      \trigger_error($this->error, $type_error_level);
      break;
    case TypecheckStatus::SERVER_BUSY:
      \trigger_error($this->error, $server_busy_level);
      break;
    case TypecheckStatus::COMMAND_NOT_FOUND:
    case TypecheckStatus::OTHER_ERROR:
      \trigger_error($this->error, $client_error_level);
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

/**
 * Typecheck the currently running endpoint with a given `hh_client`. Does some
 * caching to hopefully be pretty cheap to call, especially when there are no
 * errors and the code isn't changing. Relies on `hh_server` to poke a stamp
 * file to say "something has changed" to invalidate our cache.
 *
 * TODO Areas for future improvement:
 *  - Key the cache by endpoint/hhconfig location, so that we correctly support
 *    more than one project per HHVM instance.
 *  - Populate the cache separately from this function, so that
 *    typecheck_and_error can have a hot path that just checks "is the world
 *    clean" without paying the apc_fetch deserialization cost (which is most
 *    of the cost of this function, if I'm benchmarking correctly).
 *  - Storing an array (instead of an object) in APC might be faster, due to the
 *    way I think HHVM can optimize COW arrays.
 */
function typecheck(string $client_name = 'hh_client'): TypecheckResult {
  // Fetch times from cache and from the stamp file. Both will return "false" on
  // error (no cached time or the stamp doesn't exist). The latter will also
  // emit a warning, which we don't care about, so suppress it.
  $success = false;
  $cached_time = \apc_fetch(CacheKeys::TIME_CACHE_KEY, inout $success);
  $old = \error_reporting();
  try {
    \error_reporting(0);
    $file_time = \filemtime('/tmp/hh_server/stamp');
  } finally {
    if (\error_reporting() === 0) {
      \error_reporting($old);
    }
  }
  $time = (int)$file_time;
  // If we actually have something in cache, and the times match, use it. Note
  // that we still don't care if the stamp file actually exists -- we just treat
  // that as "time 0" (cast bool to int); it will stay zero as long as the file
  // doesn't exist and become nonzero when hh_server starts up and creates it,
  // which is what we want.
  if ($cached_time !== false && (int)$cached_time === $time) {
    $result = \apc_fetch(CacheKeys::RESULT_CACHE_KEY, inout $success);
  } else {
    $result = \__SystemLib\HH\Client\typecheck_impl($client_name);

    \apc_store(CacheKeys::TIME_CACHE_KEY, $time);
    \apc_store(CacheKeys::RESULT_CACHE_KEY, $result);
  }

  return $result;
}

/**
 * This is deliberately an unconfigurable convenience wrapper. If you want
 * full configurability, call typecheck() and use the TypecheckResult
 * yourself.
 */
function typecheck_and_error(): void {
  typecheck()->triggerError();
}

}
