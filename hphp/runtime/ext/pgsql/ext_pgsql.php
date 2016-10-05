<?hh

<<__Native>>
function pg_affected_rows(resource $result): int;

function pg_cmdtuples(resource $result): int {
  return pg_affected_rows($result);
}

<<__Native>>
function pg_cancel_query(resource $connection): bool;

<<__Native>>
function pg_client_encoding(resource $connection): ?string;

<<__Native>>
function pg_close(resource $connection): bool;

<<__Native>>
function pg_connect(
  string $connection_string,
  int $connection_type = 0,
): ?resource;

<<__Native>>
function pg_pconnect(
  string $connection_string,
  int $connection_type = 0,
): ?resource;

<<__Native>>
function pg_connection_pool_stat(): ?array;

<<__Native>>
function pg_connection_pool_sweep_free(): void;

<<__Native>>
function pg_async_connect(
  string $connection_string,
  int $connect_type = 0,
): ?resource;

<<__Native>>
function pg_connection_busy(resource $connection): bool;

<<__Native>>
function pg_connection_reset(resource $connection): bool;

<<__Native>>
function pg_connection_status(resource $connection): int;

<<__Native>>
function pg_convert(
  resource $connection,
  string $table_name,
  array<mixed> $assoc_array,
  int $option,
): mixed;

<<__Native>>
function pg_copy_from(
  resource $connection,
  string $table_name,
  array<mixed> $rows,
  string $delimiter = "\t",
  string $null_as = "\n",
): bool;

<<__Native>>
function pg_copy_to(
  resource $connection,
  string $table_name,
  string $delimiter = "\t",
  string $null_as = "\n",
): mixed;

<<__Native>>
function pg_dbname(resource $connection): ?string;

<<__Native>>
function pg_end_copy(resource $connection): bool;

<<__Native>>
function pg_escape_bytea(resource $connection, string $data): string;

<<__Native>>
function pg_escape_identifier(resource $connection, string $data): string;

<<__Native>>
function pg_escape_literal(resource $connection, string $data): string;

<<__Native>>
function pg_escape_string(resource $connection, string $data): string;

<<__Native>>
function pg_execute(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
): ?resource;

function pg_exec(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
): ?resource {
  return pg_execute($connection, $stmtname, $params);
}

<<__Native>>
function pg_fetch_all_columns(resource $result, int $column = 0): mixed;

<<__Native>>
function pg_fetch_all(resource $result): mixed;

<<__Native>>
function pg_fetch_array(
  resource $result,
  ?int $row = null,
  int $result_type = 3,
): mixed;

<<__Native>>
function pg_fetch_assoc(resource $result, ?int $row = null): mixed;

function pg_fetch_object(resource $result, ?int $row = null): mixed {
  return
    ($return = pg_fetch_assoc($result, $row)) ? (object) $return : $return;
}

<<__Native>>
function pg_fetch_result(
  resource $result,
  ?int $row = null,
  mixed $field = null,
): mixed;

<<__Native>>
function pg_fetch_row(resource $result, ?int $row = null): mixed;

<<__Native>>
function pg_field_is_null(
  resource $result,
  mixed $row,
  mixed $field = null,
): ?int;

<<__Native>>
function pg_field_name(resource $result, int $field_number): ?string;

function pg_fieldname(resource $result, int $field_number): ?string {
  return pg_field_name($result, $field_number);
}

<<__Native>>
function pg_field_num(resource $result, string $field_name): int;

<<__Native>>
function pg_field_prtlen(
  resource $result,
  mixed $row_number,
  mixed $field = null_variant,
): ?int;

<<__Native>>
function pg_field_size(resource $result, int $field_number): ?int;

function pg_fieldsize(resource $result, int $field_number): ?int {
  return pg_field_size($result, $field_number);
}

<<__Native>>
function pg_field_table(
  resource $result,
  int $field_number,
  bool $oid_only = false,
): mixed;

<<__Native>>
function pg_field_type_oid(resource $result, int $field_number): ?int;

<<__Native>>
function pg_field_type(resource $result, int $field_number): ?string;

function pg_fieldtype(resource $result, int $field_number): ?string {
  return pg_field_type($result, $field_number);
}

<<__Native>>
function pg_free_result(resource $result): bool;

function pg_freeresult(resource $result): bool {
  return pg_free_result($result);
}

<<__Native>>
function pg_get_notify(resource $connection, int $result_type = 3): mixed;

<<__Native>>
function pg_get_pid(resource $connection): int;

<<__Native>>
function pg_get_result(resource $connection): ?resource;

<<__Native>>
function pg_host(resource $connection): ?string;

<<__Native>>
function pg_last_error(resource $connection): ?string;

function pg_errormessage(resource $connection): ?string {
  return pg_last_error($connection);
}

<<__Native>>
function pg_last_notice(resource $connection): ?string;

<<__Native>>
function pg_last_oid(resource $result): mixed;

function pg_getlastoid(resource $result): mixed {
  return pg_last_oid($result);
}

<<__Native>>
function pg_meta_data(resource $connection, string $table_name): mixed;

<<__Native>>
function pg_num_fields(resource $result): int;

function pg_numfields(resource $result): int {
  return pg_num_fields($result);
}

<<__Native>>
function pg_num_rows(resource $result): int;

function pg_numrows(resource $result): int {
  return pg_num_rows($result);
}

<<__Native>>
function pg_options(resource $connection): ?string;

<<__Native>>
function pg_parameter_status(resource $connection, string $param_name): mixed;

<<__Native>>
function pg_ping(resource $connection): bool;

<<__Native>>
function pg_port(resource $connection): mixed;

<<__Native>>
function pg_prepare(
  resource $connection,
  string $stmtname,
  string $query,
): ?resource;

<<__Native>>
function pg_put_line(resource $connection, string $data): bool;

<<__Native>>
function pg_query_params(
  resource $connection,
  string $query,
  array<mixed> $params,
): ?resource;

<<__Native>>
function pg_query(resource $connection, string $query): ?resource;

<<__Native>>
function pg_result_error_field(resource $result, int $fieldcode): ?string;

<<__Native>>
function pg_result_error(resource $result): ?string;

<<__Native>>
function pg_result_seek(resource $result, int $offset): bool;

<<__Native>>
function pg_result_status(resource $result, int $type = 1): mixed;

<<__Native>>
function pg_send_execute(
  resource $connection,
  string $stmtname,
  array<mixed> $params,
): bool;

<<__Native>>
function pg_send_prepare(
  resource $connection,
  string $stmtname,
  string $query,
): bool;

<<__Native>>
function pg_send_query_params(
  resource $connection,
  string $query,
  array<mixed> $params,
): bool;

<<__Native>>
function pg_send_query(resource $connection, string $query): bool;

<<__Native>>
function pg_set_client_encoding(resource $connection, string $encoding): int;

<<__Native>>
function pg_set_error_verbosity(resource $connection, int $verbosity): mixed;

<<__Native>>
function pg_trace(string $pathname, string $mode, resource $connection): bool;

<<__Native>>
function pg_transaction_status(resource $connection): int;

<<__Native>>
function pg_unescape_bytea(string $data): string;

<<__Native>>
function pg_untrace(resource $connection): bool;

<<__Native>>
function pg_version(resource $connection): ?array;
