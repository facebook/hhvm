<?hh // decl

const int PGSQL_ASSOC = 0;
const int PGSQL_NUM = 0;
const int PGSQL_BOTH = 0;

const int PGSQL_CONNECT_FORCE_NEW = 0;
const int PGSQL_CONNECTION_BAD = 0;
const int PGSQL_CONNECTION_OK = 0;
const int PGSQL_CONNECTION_STARTED = 0;
const int PGSQL_CONNECTION_MADE = 0;
const int PGSQL_CONNECTION_AWAITING_RESPONSE = 0;
const int PGSQL_CONNECTION_AUTH_OK = 0;
const int PGSQL_CONNECTION_SETENV = 0;
const int PGSQL_CONNECTION_SSL_STARTUP = 0;

const int PGSQL_SEEK_SET = 0;
const int PGSQL_SEEK_CUR = 0;
const int PGSQL_SEEK_END = 0;

const int PGSQL_EMPTY_QUERY = 0;
const int PGSQL_COMMAND_OK = 0;
const int PGSQL_TUPLES_OK = 0;
const int PGSQL_COPY_OUT = 0;
const int PGSQL_COPY_IN = 0;
const int PGSQL_BAD_RESPONSE = 0;
const int PGSQL_NONFATAL_ERROR = 0;
const int PGSQL_FATAL_ERROR = 0;

const int PGSQL_TRANSACTION_IDLE = 0;
const int PGSQL_TRANSACTION_ACTIVE = 0;
const int PGSQL_TRANSACTION_INTRANS = 0;
const int PGSQL_TRANSACTION_INERROR = 0;
const int PGSQL_TRANSACTION_UNKNOWN = 0;

const int PGSQL_DIAG_SEVERITY = 0;
const int PGSQL_DIAG_SQLSTATE = 0;
const int PGSQL_DIAG_MESSAGE_PRIMARY = 0;
const int PGSQL_DIAG_MESSAGE_DETAIL = 0;
const int PGSQL_DIAG_MESSAGE_HINT = 0;
const int PGSQL_DIAG_STATEMENT_POSITION = 0;
const int PGSQL_DIAG_INTERNAL_POSITION = 0;
const int PGSQL_DIAG_INTERNAL_QUERY = 0;
const int PGSQL_DIAG_CONTEXT = 0;
const int PGSQL_DIAG_SOURCE_FILE = 0;
const int PGSQL_DIAG_SOURCE_LINE = 0;
const int PGSQL_DIAG_SOURCE_FUNCTION = 0;

const int PGSQL_ERRORS_TERSE = 0;
const int PGSQL_ERRORS_DEFAULT = 0;
const int PGSQL_ERRORS_VERBOSE = 0;

const int PGSQL_STATUS_LONG = 0;
const int PGSQL_STATUS_STRING = 0;

const int PGSQL_CONV_IGNORE_DEFAULT = 0;
const int PGSQL_CONV_FORCE_NULL = 0;
const int PGSQL_CONV_IGNORE_NOT_NULL = 0;

function pg_affected_rows(resource $result): int;
function pg_cmdtuples(resource $result): int;
function pg_cancel_query(resource $connection): bool;
function pg_client_encoding(resource $connection);
function pg_close(resource $connection): bool;
function pg_connect(string $connection_string, int $connection_type = 0);
function pg_pconnect(string $connection_string, int $connection_type = 0);
function pg_connection_pool_stat(): array<array>;
function pg_connection_pool_sweep_free(): void;
function pg_connection_busy(resource $connection): bool;
function pg_connection_reset(resource $connection): bool;
function pg_connection_status(resource $connection): int;
function pg_dbname(resource $connection);
function pg_escape_bytea(resource $connection, string $data): ?string;
function pg_escape_identifier(resource $connection, string $data): ?string;
function pg_escape_literal(resource $connection, string $data): ?string;
function pg_escape_string(resource $connection, string $data): ?string;
function pg_execute(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
);
function pg_exec(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
): ?resource;
function pg_fetch_all_columns(resource $result, int $column = 0);
function pg_fetch_all(resource $result);
function pg_fetch_array(
  resource $result,
  ?int $row = null,
  int $result_type = 3,
);
function pg_fetch_assoc(resource $result, ?int $row = null);
function pg_fetch_object(resource $result, ?int $row = null);
function pg_fetch_result(
  resource $result,
  ?int $row = null,
  mixed $field = null,
);
function pg_fetch_row(resource $result, ?int $row = null);
function pg_field_is_null(resource $result, mixed $row, mixed $field = null);
function pg_field_name(resource $result, int $field_number);
function pg_fieldname(resource $result, int $field_number);
function pg_field_num(resource $result, string $field_name): int;
function pg_field_prtlen(
  resource $result,
  mixed $row_number,
  mixed $field = uninit_variant,
);
function pg_field_size(resource $result, int $field_number);
function pg_fieldsize(resource $result, int $field_number);
function pg_field_table(
  resource $result,
  int $field_number,
  bool $oid_only = false,
);
function pg_field_type_oid(resource $result, int $field_number);
function pg_field_type(resource $result, int $field_number);
function pg_fieldtype(resource $result, int $field_number);
function pg_free_result(resource $result): bool;
function pg_freeresult(resource $result): bool;
function pg_get_pid(resource $connection): int;
function pg_get_result(resource $connection);
function pg_host(resource $connection);
function pg_last_error(resource $connection);
function pg_errormessage(resource $connection);
function pg_last_notice(resource $connection);
function pg_last_oid(resource $result);
function pg_getlastoid(resource $result);
function pg_num_fields(resource $result): int;
function pg_numfields(resource $result): int;
function pg_num_rows(resource $result): int;
function pg_numrows(resource $result): int;
function pg_options(resource $connection);
function pg_parameter_status(resource $connection, string $param_name);
function pg_ping(resource $connection): bool;
function pg_port(resource $connection);
function pg_prepare(resource $connection, string $stmtname, string $query);
function pg_query_params(
  resource $connection,
  string $query,
  array<mixed> $params,
);
function pg_query(resource $connection, string $query);
function pg_result_error_field(resource $result, int $fieldcode);
function pg_result_error(resource $result);
function pg_result_seek(resource $result, int $offset): bool;
function pg_result_status(resource $result, int $type = 1): arraykey;
function pg_send_execute(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
): bool;
function pg_send_prepare(
  resource $connection,
  string $stmtname,
  string $query,
): bool;
function pg_send_query_params(
  resource $connection,
  string $query,
  array<mixed> $params,
): bool;
function pg_send_query(resource $connection, string $query): bool;
function pg_set_client_encoding(resource $connection, string $encoding): int;
function pg_set_error_verbosity(resource $connection, int $verbosity);
function pg_transaction_status(resource $connection): int;
function pg_unescape_bytea(string $data): string;
function pg_version(resource $connection);
