<?hh // decl

const int MYSQLI_READ_DEFAULT_GROUP = 5;
const int MYSQLI_READ_DEFAULT_FILE = 4;
const int MYSQLI_OPT_CONNECT_TIMEOUT = 0;
const int MYSQLI_OPT_LOCAL_INFILE = 8;
const int MYSQLI_SERVER_PUBLIC_KEY = 27;
const int MYSQLI_INIT_COMMAND = 3;
const int MYSQLI_OPT_NET_CMD_BUFFER_SIZE = 202;
const int MYSQLI_OPT_NET_READ_BUFFER_SIZE = 203;
const int MYSQLI_OPT_INT_AND_FLOAT_NATIVE = 201;
const int MYSQLI_CLIENT_SSL = 2048;
const int MYSQLI_CLIENT_COMPRESS = 32;
const int MYSQLI_CLIENT_INTERACTIVE = 1024;
const int MYSQLI_CLIENT_IGNORE_SPACE = 256;
const int MYSQLI_CLIENT_NO_SCHEMA = 16;
const int MYSQLI_CLIENT_FOUND_ROWS = 2;
const int MYSQLI_STORE_RESULT = 0;
const int MYSQLI_USE_RESULT = 1;
const int MYSQLI_ASYNC = 8;
const int MYSQLI_ASSOC = 1;
const int MYSQLI_NUM = 2;
const int MYSQLI_BOTH = 3;
const int MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH = 0;
const int MYSQLI_STMT_ATTR_CURSOR_TYPE = 1;
const int MYSQLI_CURSOR_TYPE_NO_CURSOR = 0;
const int MYSQLI_CURSOR_TYPE_READ_ONLY = 1;
const int MYSQLI_CURSOR_TYPE_FOR_UPDATE = 2;
const int MYSQLI_CURSOR_TYPE_SCROLLABLE = 4;
const int MYSQLI_STMT_ATTR_PREFETCH_ROWS = 2;
const int MYSQLI_NOT_NULL_FLAG = 1;
const int MYSQLI_PRI_KEY_FLAG = 2;
const int MYSQLI_UNIQUE_KEY_FLAG = 4;
const int MYSQLI_MULTIPLE_KEY_FLAG = 8;
const int MYSQLI_BLOB_FLAG = 16;
const int MYSQLI_UNSIGNED_FLAG = 32;
const int MYSQLI_ZEROFILL_FLAG = 64;
const int MYSQLI_AUTO_INCREMENT_FLAG = 512;
const int MYSQLI_TIMESTAMP_FLAG = 1024;
const int MYSQLI_SET_FLAG = 2048;
const int MYSQLI_NUM_FLAG = 32768;
const int MYSQLI_PART_KEY_FLAG = 16384;
const int MYSQLI_GROUP_FLAG = 32768;
const int MYSQLI_ENUM_FLAG = 256;
const int MYSQLI_BINARY_FLAG = 128;
const int MYSQLI_NO_DEFAULT_VALUE_FLAG = 4096;
const int MYSQLI_ON_UPDATE_NOW_FLAG = 8192;
const int MYSQLI_TYPE_DECIMAL = 0;
const int MYSQLI_TYPE_TINY = 1;
const int MYSQLI_TYPE_SHORT = 2;
const int MYSQLI_TYPE_LONG = 3;
const int MYSQLI_TYPE_FLOAT = 4;
const int MYSQLI_TYPE_DOUBLE = 5;
const int MYSQLI_TYPE_NULL = 6;
const int MYSQLI_TYPE_TIMESTAMP = 7;
const int MYSQLI_TYPE_LONGLONG = 8;
const int MYSQLI_TYPE_INT24 = 9;
const int MYSQLI_TYPE_DATE = 10;
const int MYSQLI_TYPE_TIME = 11;
const int MYSQLI_TYPE_DATETIME = 12;
const int MYSQLI_TYPE_YEAR = 13;
const int MYSQLI_TYPE_NEWDATE = 14;
const int MYSQLI_TYPE_ENUM = 247;
const int MYSQLI_TYPE_SET = 248;
const int MYSQLI_TYPE_TINY_BLOB = 249;
const int MYSQLI_TYPE_MEDIUM_BLOB = 250;
const int MYSQLI_TYPE_LONG_BLOB = 251;
const int MYSQLI_TYPE_BLOB = 252;
const int MYSQLI_TYPE_VAR_STRING = 253;
const int MYSQLI_TYPE_STRING = 254;
const int MYSQLI_TYPE_CHAR = 1;
const int MYSQLI_TYPE_INTERVAL = 247;
const int MYSQLI_TYPE_GEOMETRY = 255;
const int MYSQLI_TYPE_NEWDECIMAL = 246;
const int MYSQLI_TYPE_BIT = 16;
const int MYSQLI_SET_CHARSET_NAME = 7;
const int MYSQLI_NO_DATA = 100;
const int MYSQLI_DATA_TRUNCATED = 101;
const int MYSQLI_REPORT_INDEX = 4;
const int MYSQLI_REPORT_ERROR = 1;
const int MYSQLI_REPORT_STRICT = 2;
const int MYSQLI_REPORT_ALL = 255;
const int MYSQLI_REPORT_OFF = 0;
const int MYSQLI_DEBUG_TRACE_ENABLED = 1;
const int MYSQLI_SERVER_QUERY_NO_GOOD_INDEX_USED = 16;
const int MYSQLI_SERVER_QUERY_NO_INDEX_USED = 32;
const int MYSQLI_REFRESH_GRANT = 1;
const int MYSQLI_REFRESH_LOG = 2;
const int MYSQLI_REFRESH_TABLES = 4;
const int MYSQLI_REFRESH_HOSTS = 8;
const int MYSQLI_REFRESH_STATUS = 16;
const int MYSQLI_REFRESH_THREADS = 32;
const int MYSQLI_REFRESH_SLAVE = 64;
const int MYSQLI_REFRESH_MASTER = 128;
const int MYSQLI_SERVER_QUERY_WAS_SLOW = 1024;
const int MYSQLI_REFRESH_BACKUP_LOG = 2097152;

final class mysqli_driver  {
  public string $client_info;
  public string $client_version;
  public string $driver_version;
  public string $embedded;
  public bool $reconnect;
  public int $report_mode;

  public function __construct() {}
}

class mysqli  {
  public int $affected_rows;
  public string $client_info;
  public int $connect_errno;
  public string $connect_error;
  public int $errno;
  public array $error_list;
  public string $error;
  public int $field_count;
  public int $client_version;
  public string $host_info;
  public string $protocol_version;
  public string $server_info;
  public int $server_version;
  public string $info;
  public mixed $insert_id;
  public string $sqlstate;
  public int $thread_id;
  public int $warning_count;

  public function __construct (
    ?string $host = null,
    ?string $username = null,
    ?string $passwd = null,
    ?string $dbname = null,
    mixed $port = null,
    ?string $socket = null
  ) {}
  public function autocommit (bool $mode): bool {}
  public function begin_transaction (?int $flags = null, ?string $name = null): bool {}
  public function change_user (string $user, string $password, string $database): bool {}
  public function character_set_name (): string {}
  public function close (): bool {}
  public function commit (int $flags = 0, ?string $name = null): bool {}
  public function connect (
    ?string $host = null,
    ?string $user = null,
    ?string $password = null,
    ?string $database = null,
    mixed $port = null,
    ?string $socket = null
  ): void {}
  public function dump_debug_info (): bool {}
  public function get_charset (): stdClass {}
  public function get_client_info (): string {}
  public function get_server_info (): ?string {}
  public function get_warnings (): mysqli_warning {}
  public function init (): mysqli {}
  public function kill (int $processid): bool {}
  public function multi_query (string $query): ?bool {}
  public function more_results (): bool {}
  public function next_result (): ?bool {}
  public function options (int $option, mixed $value): bool {}
  public function ping (): bool {}
  public function prepare (string $query): mixed {}
  public function query (string $query, int $resultmode = MYSQLI_STORE_RESULT) {}
  public function real_connect (
    ?string $host = null,
    ?string $username = null,
    ?string $passwd = null,
    ?string $dbname = null,
    mixed $port = null,
    ?string $socket = null,
    ?int $flags = 0
  ): bool {}
  public function real_escape_string (string $escapestr): ?string {}
  public function escape_string (string $escapestr): ?string {}
  public function real_query (string $query): ?bool {}
  public function release_savepoint (string $name): ?bool {}
  public function rollback (int $flags = 0, ?string $name = null): ?bool {}
  public function savepoint (string $name): ?bool {}
  public function select_db (string $dbname): ?bool {}
  public function set_charset (string $charset): bool {}
  public function set_opt (int $option, mixed $value): bool {}
  public function ssl_set(
    ?string $key,
    ?string $cert,
    ?string $ca,
    ?string $capath,
    ?string $cipher): bool {}
  public function stat (): ?string {}
  public function stmt_init (): mysqli_stmt {}
  public function store_result (): mixed {}
  public function use_result (): mixed {}
  public function refresh (int $options): bool {}
}

final class mysqli_warning  {
  public string $message;
  public string $sqlstate;
  public int $errno;

  protected function __construct (array $warnings) {}
  public function next (): bool {}
}

class mysqli_result {
  public int $current_field;
  public int $field_count;
  public array $lengths;
  public int $num_rows;
  public function __construct (mixed $result, int $resulttype = MYSQLI_STORE_RESULT) {}
  public function close (): void {}
  public function data_seek (int $offset): mixed {}
  public function fetch_field (): mixed {}
  public function fetch_fields (): array {}
  public function fetch_field_direct (int $fieldnr): mixed {}
  public function fetch_all (int $resulttype = MYSQLI_NUM): mixed {}
  public function fetch_array (int $resulttype = MYSQLI_BOTH): mixed {}
  public function fetch_assoc (): mixed {}
  public function fetch_object (? string $class_name = null, array $params = array()): mixed {}
  public function fetch_row (): mixed {}
  public function field_seek (int $fieldnr): bool {}
  public function free (): void {}
  public function free_result (): void {}
}

class mysqli_stmt {
  public int $affected_rows;
  public int $insert_id;
  public int $num_rows;
  public int $param_count;
  public int $field_count;
  public int $errno;
  public string $error;
  public string $sqlstate;
  public int $id;
  public function __construct (mysqli $link, ?string $query = null) {}
  public function attr_get (int $attr): mixed {}
  public function attr_set (int $attr, int $mode): bool {}
  public function bind_param (string $types, ...): bool {}
  public function bind_result (...): bool {}
  public function close (): bool {}
  public function data_seek (int $offset): void {}
  public function execute (): bool {}
  public function fetch (): ?bool {}
  public function get_warnings (): mixed {}
  public function result_metadata (): mixed {}
  public function num_rows (mysqli_stmt $stmt) {}
  public function send_long_data (int $param_nr, string $data): mixed {}
  public function free_result (): void {}
  public function reset (): bool {}
  public function prepare (string $query): mixed {}
  public function store_result (): bool {}
}

function mysqli_affected_rows (mysqli $link): ?int {}
function mysqli_autocommit (mysqli $link, bool $mode): bool {}
function mysqli_begin_transaction (
  mysqli $link,
  int $flags = 0,
  ?string $name = null): bool {}
function mysqli_change_user (
  mysqli $link,
  string $user,
  string $password,
  string $database): bool {}
function mysqli_character_set_name (mysqli $link): string {}
function mysqli_close (mysqli $link): bool {}
function mysqli_commit (mysqli $link, int $flags = 0, ?string $name = null): bool {}
function mysqli_connect (
  ?string $host = null,
  ?string $user = null,
  ?string $password = null,
  ?string $database = null,
  mixed $port = null,
  ?string $socket = null): mixed {}
function mysqli_connect_errno (): int {}
function mysqli_connect_error (): ?string {}
function mysqli_data_seek (mysqli_result $result, int $offset): bool {}
function mysqli_dump_debug_info (mysqli $link): bool {}
function mysqli_errno ($link): ?int {}
function mysqli_error_list (mysqli $link): array {}
function mysqli_stmt_error_list (mysqli_stmt $stmt): array {}
function mysqli_error (mysqli $link): ?string {}
function mysqli_stmt_execute (mysqli_stmt $stmt): bool {}
function mysqli_fetch_field (mysqli_result $result): mixed {}
function mysqli_fetch_fields (mysqli_result $result): array {}
function mysqli_fetch_field_direct (mysqli_result $result, int $fieldnr): mixed {}
function mysqli_fetch_lengths (mysqli_result $result): array {}
function mysqli_fetch_all (mysqli_result $result, int $resulttype = MYSQLI_NUM): mixed {}
function mysqli_fetch_array (mysqli_result $result, int $resulttype = MYSQLI_BOTH): mixed {}
function mysqli_fetch_assoc (mysqli_result $result): mixed {}
function mysqli_fetch_object (
  mysqli_result $result,
  ?string $class_name = null,
  ?array $params = array()): mixed {}
function mysqli_fetch_row (mysqli_result $result): ?array<string> {}
function mysqli_field_count (mysqli $link): ?int {}
function mysqli_field_seek (mysqli_result $result, int $fieldnr): bool {}
function mysqli_field_tell (mysqli_result $result): int {}
function mysqli_free_result (mysqli_result $result): void {}
function mysqli_get_charset (mysqli $link): stdClass {}
function mysqli_get_client_info (): string {}
function mysqli_get_client_version (mysqli $link): int {}
function mysqli_get_host_info (mysqli $link): ?string {}
function mysqli_get_proto_info (mysqli $link): int {}
function mysqli_get_server_info (mysqli $link): ?string {}
function mysqli_get_server_version (mysqli $link): ?int {}
function mysqli_get_warnings (mysqli $link): mysqli_warning {}
function mysqli_init (): mysqli {}
function mysqli_info (mysqli $link): ?string {}
function mysqli_insert_id (mysqli $link): mixed {}
function mysqli_kill (mysqli $link, int $processid): bool {}
function mysqli_more_results (mysqli $link): bool {}
function mysqli_multi_query (mysqli $link, string $query): bool {}
function mysqli_next_result (mysqli $link): bool {}
function mysqli_num_fields (mysqli_result $result): ?int {}
function mysqli_num_rows (mysqli_result $result): int {}
function mysqli_options (mysqli $link, int $option, mixed $value): bool {}
function mysqli_ping (mysqli $link): bool {}
function mysqli_prepare (mysqli $link, string $query): mixed {}
function mysqli_report (int $flags): bool {}
function mysqli_query (
  mysqli $link,
  string $query,
  int $resultmode = MYSQLI_STORE_RESULT): mixed {}
function mysqli_real_connect (
  mysqli $link,
  ?string $host = null,
  ?string $user = null,
  ?string $password = null,
  ?string $database = null,
  mixed $port = null,
  ?string $socket = null,
  int $flags = 0): bool {}
function mysqli_real_escape_string (mysqli $link, string $escapestr): ?string {}
function mysqli_real_query (mysqli $link, string $query): bool {}
function mysqli_release_savepoint (mysqli $link, string $name): bool {}
function mysqli_rollback(mysqli $link, int $flags = 0, ?string $name = null): ?bool {}
function mysqli_savepoint(mysqli $link, string $name): bool {}
function mysqli_select_db(mysqli $link, string $dbname): ?bool {}
function mysqli_set_charset(mysqli $link, string $charset): bool {}
function mysqli_stmt_affected_rows(mysqli_stmt $stmt): ?int {}
function mysqli_stmt_attr_get(mysqli_stmt $stmt, int $attr): mixed {}
function mysqli_stmt_attr_set(mysqli_stmt $stmt, int $attr, int $mode): bool {}
function mysqli_stmt_field_count (mysqli $stmt): ?int {}
function mysqli_stmt_init (mysqli $link): mysqli_stmt {}
function mysqli_stmt_prepare (mysqli_stmt $stmt, string $query): bool {}
function mysqli_stmt_result_metadata(mysqli_stmt $stmt): mixed {}
function mysqli_stmt_send_long_data (mysqli_stmt $stmt, int $param_nr, string $data): bool {}
function mysqli_stmt_bind_param(mysqli_stmt $stmt, string $types, ...): bool {}
function mysqli_stmt_bind_result (mysqli_stmt $stmt, ...): bool {}
function mysqli_stmt_fetch(mysqli_stmt $stmt): ?bool {}
function mysqli_stmt_free_result (mysqli_stmt $stmt): void {}
function mysqli_stmt_get_warnings (mysqli_stmt $stmt): mixed {}
function mysqli_stmt_insert_id (mysqli_stmt $stmt): mixed {}
function mysqli_stmt_reset (mysqli_stmt $stmt): bool {}
function mysqli_stmt_param_count (mysqli_stmt $stmt): int {}
function mysqli_sqlstate (mysqli $link): ?string {}
function mysqli_stat (mysqli $link): ?string {}
function mysqli_stmt_close (mysqli_stmt $stmt): bool {}
function mysqli_stmt_data_seek (mysqli_stmt $stmt, int $offset): void {}
function mysqli_stmt_errno (mysqli_stmt $stmt): ?int {}
function mysqli_stmt_error (mysqli_stmt $stmt): ?string {}
function mysqli_stmt_more_results (mysqli_stmt $stmt): bool {}
function mysqli_stmt_next_result (mysqli_stmt $stmt): bool {}
function mysqli_stmt_num_rows (mysqli_stmt $stmt): int {}
function mysqli_stmt_sqlstate (mysqli_stmt $stmt): ?string {}
function mysqli_stmt_store_result (mysqli_stmt $stmt): bool {}
function mysqli_store_result (mysqli $link): mixed {}
function mysqli_thread_id (mysqli $link): ?int {}
function mysqli_thread_safe (): bool {}
function mysqli_use_result (mysqli $link): mixed {}
function mysqli_warning_count (mysqli $link): ?int {}
function mysqli_refresh (mysqli $link, int $options): bool {}
function mysqli_escape_string (mysqli $link, string $query): ?string {}
function mysqli_set_opt (mysqli $link, int $option, mixed $value): bool {}
