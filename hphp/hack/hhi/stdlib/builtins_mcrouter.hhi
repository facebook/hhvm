<?hh

<<__PHPStdLib>>
class MCRouterException extends Exception {
  public function getKey(): string;
  public function getOp(): int;

  public function __construct(
    string $message,
    protected int $op = MCRouter::mc_op_unknown,
    int $reply = MCRouter::mc_res_unknown,
    protected string $key = "",
  );
}

<<__PHPStdLib>>
class MCRouterOptionException extends Exception {
  public function getErrors(): varray<darray<string, string>>;
  public function __construct(protected varray<darray<string, string>> $errors);
}

<<__PHPStdLib>>
class MCRouter {
  public function __construct(
    darray<string, mixed> $options,
    string $pid = "",
  ): void;
  public static function createSimple(ConstVector<string> $servers): MCRouter;
  public async function add(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;
  public async function set(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;
  public async function replace(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;
  public async function prepend(string $key, string $value): Awaitable<void>;
  public async function append(string $key, string $value): Awaitable<void>;
  public async function incr(string $key, int $val): Awaitable<int>;
  public async function decr(string $key, int $val): Awaitable<int>;
  public async function del(string $key): Awaitable<void>;
  public async function flushAll(int $delay = 0): Awaitable<void>;
  public async function get(string $key): Awaitable<string>;
  public async function gets(string $key): Awaitable<shape(
    'value' => string,
    'cas' => int,
    'flags' => int,
  )>;
  public async function version(): Awaitable<string>;
  public static function getOpName(int $op): string;
  public static function getResultName(int $op): string;

  public async function cas(
    int $cas,
    string $key,
    string $value,
    int $expiration = 0,
  ): Awaitable<void>;

  // From mcrouter/lib/mc/msg.h enum mc_op_e:
  const int mc_op_unknown;
  const int mc_op_echo;
  const int mc_op_quit;
  const int mc_op_version;
  const int mc_op_servererr;
  const int mc_op_get;
  const int mc_op_set;
  const int mc_op_add;
  const int mc_op_replace;
  const int mc_op_append;
  const int mc_op_prepend;
  const int mc_op_cas;
  const int mc_op_delete;
  const int mc_op_incr;
  const int mc_op_decr;
  const int mc_op_flushall;
  const int mc_op_flushre;
  const int mc_op_stats;
  const int mc_op_verbosity;
  const int mc_op_lease_get;
  const int mc_op_lease_set;
  const int mc_op_shutdown;
  const int mc_op_end;
  const int mc_op_metaget;
  const int mc_op_exec;
  const int mc_op_gets;
  const int mc_op_get_service_info;

  // From mcrouter/lib/carbon/Result.h enum carbon::Result:
  const int mc_res_unknown;
  const int mc_res_deleted;
  const int mc_res_found;
  const int mc_res_foundstale;
  const int mc_res_notfound;
  const int mc_res_notfoundhot;
  const int mc_res_notstored;
  const int mc_res_stalestored;
  const int mc_res_ok;
  const int mc_res_stored;
  const int mc_res_exists;
  const int mc_res_ooo;
  const int mc_res_timeout;
  const int mc_res_connect_timeout;
  const int mc_res_connect_error;
  const int mc_res_busy;
  const int mc_res_try_again;
  const int mc_res_tko;
  const int mc_res_bad_command;
  const int mc_res_bad_key;
  const int mc_res_bad_flags;
  const int mc_res_bad_exptime;
  const int mc_res_bad_lease_id;
  const int mc_res_bad_cas_id;
  const int mc_res_bad_value;
  const int mc_res_aborted;
  const int mc_res_client_error;
  const int mc_res_local_error;
  const int mc_res_remote_error;
  const int mc_res_waiting;
}
