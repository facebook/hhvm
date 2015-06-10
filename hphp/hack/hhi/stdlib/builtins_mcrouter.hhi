<?hh // decl

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

class MCRouterOptionException extends Exception {
  public function getErrors(): array<array<string,string>>;
  public function __construct(protected array<array<string,string>> $errors);
}

class MCRouter {
  public function __construct(
    array<string,mixed> $options,
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
  public async function version(): Awaitable<string>;
  public static function getOpName(int $op): string;
  public static function getResultName(int $op): string;

  // From mcrouter/lib/mc/msg.h enum mc_op_e:
  const int mc_op_unknown = 0;
  const int mc_op_echo = 0;
  const int mc_op_quit = 0;
  const int mc_op_version = 0;
  const int mc_op_servererr = 0;
  const int mc_op_get = 0;
  const int mc_op_set = 0;
  const int mc_op_add = 0;
  const int mc_op_replace = 0;
  const int mc_op_append = 0;
  const int mc_op_prepend = 0;
  const int mc_op_cas = 0;
  const int mc_op_delete = 0;
  const int mc_op_incr = 0;
  const int mc_op_decr = 0;
  const int mc_op_flushall = 0;
  const int mc_op_flushre = 0;
  const int mc_op_stats = 0;
  const int mc_op_verbosity = 0;
  const int mc_op_lease_get = 0;
  const int mc_op_lease_set = 0;
  const int mc_op_shutdown = 0;
  const int mc_op_end = 0;
  const int mc_op_metaget = 0;
  const int mc_op_exec = 0;
  const int mc_op_gets = 0;
  const int mc_op_get_service_info = 0;

  // From mcrouter/lib/mc/msg.h enum mc_res_e:
  const int mc_res_unknown = 0;
  const int mc_res_deleted = 0;
  const int mc_res_found = 0;
  const int mc_res_foundstale = 0;
  const int mc_res_notfound = 0;
  const int mc_res_notfoundhot = 0;
  const int mc_res_notstored = 0;
  const int mc_res_stalestored = 0;
  const int mc_res_ok = 0;
  const int mc_res_stored = 0;
  const int mc_res_exists = 0;
  const int mc_res_ooo = 0;
  const int mc_res_timeout = 0;
  const int mc_res_connect_timeout = 0;
  const int mc_res_connect_error = 0;
  const int mc_res_busy = 0;
  const int mc_res_try_again = 0;
  const int mc_res_tko = 0;
  const int mc_res_bad_command = 0;
  const int mc_res_bad_key = 0;
  const int mc_res_bad_flags = 0;
  const int mc_res_bad_exptime = 0;
  const int mc_res_bad_lease_id = 0;
  const int mc_res_bad_cas_id = 0;
  const int mc_res_bad_value = 0;
  const int mc_res_aborted = 0;
  const int mc_res_client_error = 0;
  const int mc_res_local_error = 0;
  const int mc_res_remote_error = 0;
  const int mc_res_waiting = 0;
}
