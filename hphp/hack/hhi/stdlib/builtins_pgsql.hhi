<?hh

const int PGSQL_ASSOC;
const int PGSQL_NUM;
const int PGSQL_BOTH;

const int PGSQL_CONNECT_FORCE_NEW;
const int PGSQL_CONNECTION_BAD;
const int PGSQL_CONNECTION_OK;
const int PGSQL_CONNECTION_STARTED;
const int PGSQL_CONNECTION_MADE;
const int PGSQL_CONNECTION_AWAITING_RESPONSE;
const int PGSQL_CONNECTION_AUTH_OK;
const int PGSQL_CONNECTION_SETENV;
const int PGSQL_CONNECTION_SSL_STARTUP;

const int PGSQL_SEEK_SET;
const int PGSQL_SEEK_CUR;
const int PGSQL_SEEK_END;

const int PGSQL_EMPTY_QUERY;
const int PGSQL_COMMAND_OK;
const int PGSQL_TUPLES_OK;
const int PGSQL_COPY_OUT;
const int PGSQL_COPY_IN;
const int PGSQL_BAD_RESPONSE;
const int PGSQL_NONFATAL_ERROR;
const int PGSQL_FATAL_ERROR;

const int PGSQL_TRANSACTION_IDLE;
const int PGSQL_TRANSACTION_ACTIVE;
const int PGSQL_TRANSACTION_INTRANS;
const int PGSQL_TRANSACTION_INERROR;
const int PGSQL_TRANSACTION_UNKNOWN;

const int PGSQL_DIAG_SEVERITY;
const int PGSQL_DIAG_SQLSTATE;
const int PGSQL_DIAG_MESSAGE_PRIMARY;
const int PGSQL_DIAG_MESSAGE_DETAIL;
const int PGSQL_DIAG_MESSAGE_HINT;
const int PGSQL_DIAG_STATEMENT_POSITION;
const int PGSQL_DIAG_INTERNAL_POSITION;
const int PGSQL_DIAG_INTERNAL_QUERY;
const int PGSQL_DIAG_CONTEXT;
const int PGSQL_DIAG_SOURCE_FILE;
const int PGSQL_DIAG_SOURCE_LINE;
const int PGSQL_DIAG_SOURCE_FUNCTION;

const int PGSQL_ERRORS_TERSE;
const int PGSQL_ERRORS_DEFAULT;
const int PGSQL_ERRORS_VERBOSE;

const int PGSQL_STATUS_LONG;
const int PGSQL_STATUS_STRING;

const int PGSQL_CONV_IGNORE_DEFAULT;
const int PGSQL_CONV_FORCE_NULL;
const int PGSQL_CONV_IGNORE_NOT_NULL;

<<__PHPStdLib>>
function pg_affected_rows(resource $result): int;
<<__PHPStdLib>>
function pg_cmdtuples(resource $result): int;
<<__PHPStdLib>>
function pg_cancel_query(resource $connection): bool;
<<__PHPStdLib>>
function pg_client_encoding(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_close(resource $connection): bool;
<<__PHPStdLib>>
function pg_connect(
  string $connection_string,
  int $connection_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_pconnect(
  string $connection_string,
  int $connection_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_connection_pool_stat(): varray<mixed>;
<<__PHPStdLib>>
function pg_connection_pool_sweep_free(): void;
<<__PHPStdLib>>
function pg_connection_busy(resource $connection): bool;
<<__PHPStdLib>>
function pg_connection_reset(resource $connection): bool;
<<__PHPStdLib>>
function pg_connection_status(resource $connection): int;
<<__PHPStdLib>>
function pg_dbname(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_escape_bytea(resource $connection, string $data): ?string;
<<__PHPStdLib>>
function pg_escape_identifier(resource $connection, string $data): ?string;
<<__PHPStdLib>>
function pg_escape_literal(resource $connection, string $data): ?string;
<<__PHPStdLib>>
function pg_escape_string(resource $connection, string $data): ?string;
<<__PHPStdLib>>
function pg_execute(
  resource $connection,
  string $stmtname,
  varray<mixed> $params,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_exec(
  resource $connection,
  string $stmtname,
  varray<mixed> $params,
): ?resource;
<<__PHPStdLib>>
function pg_fetch_all_columns(
  resource $result,
  int $column = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fetch_all(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fetch_array(
  resource $result,
  ?int $row = null,
  int $result_type = 3,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fetch_assoc(
  resource $result,
  ?int $row = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fetch_result(
  resource $result,
  ?int $row = null,
  mixed $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fetch_row(
  resource $result,
  ?int $row = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_is_null(
  resource $result,
  mixed $row,
  mixed $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_name(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fieldname(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_num(resource $result, string $field_name): int;
<<__PHPStdLib>>
function pg_field_prtlen(
  resource $result,
  mixed $row_number,
  mixed $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_size(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fieldsize(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_table(
  resource $result,
  int $field_number,
  bool $oid_only = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_type_oid(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_field_type(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_fieldtype(
  resource $result,
  int $field_number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_free_result(resource $result): bool;
<<__PHPStdLib>>
function pg_freeresult(resource $result): bool;
<<__PHPStdLib>>
function pg_get_pid(resource $connection): int;
<<__PHPStdLib>>
function pg_get_result(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_host(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_last_error(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_errormessage(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_last_notice(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_last_oid(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_getlastoid(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_num_fields(resource $result): int;
<<__PHPStdLib>>
function pg_numfields(resource $result): int;
<<__PHPStdLib>>
function pg_num_rows(resource $result): int;
<<__PHPStdLib>>
function pg_numrows(resource $result): int;
<<__PHPStdLib>>
function pg_options(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_parameter_status(
  resource $connection,
  string $param_name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_ping(resource $connection): bool;
<<__PHPStdLib>>
function pg_port(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_prepare(
  resource $connection,
  string $stmtname,
  string $query,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_query_params(
  resource $connection,
  string $query,
  varray<mixed> $params,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_query(
  resource $connection,
  string $query,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_result_error_field(
  resource $result,
  int $fieldcode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_result_error(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_result_seek(resource $result, int $offset): bool;
<<__PHPStdLib>>
function pg_result_status(resource $result, int $type = 1): arraykey;
<<__PHPStdLib>>
function pg_send_execute(
  resource $connection,
  string $stmtname,
  varray<mixed> $params,
): bool;
<<__PHPStdLib>>
function pg_send_prepare(
  resource $connection,
  string $stmtname,
  string $query,
): bool;
<<__PHPStdLib>>
function pg_send_query_params(
  resource $connection,
  string $query,
  varray<mixed> $params,
): bool;
<<__PHPStdLib>>
function pg_send_query(resource $connection, string $query): bool;
<<__PHPStdLib>>
function pg_set_client_encoding(resource $connection, string $encoding): int;
<<__PHPStdLib>>
function pg_set_error_verbosity(
  resource $connection,
  int $verbosity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pg_transaction_status(resource $connection): int;
<<__PHPStdLib>>
function pg_unescape_bytea(string $data): string;
<<__PHPStdLib>>
function pg_version(resource $connection): HH\FIXME\MISSING_RETURN_TYPE;
