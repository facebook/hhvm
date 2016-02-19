<?hh // decl

function pg_affected_rows(resource $result): int;

function pg_cancel_query(resource $connection): bool;

function pg_client_encoding(resource $connection): ?string;

function pg_close(resource $connection): bool;

function pg_connect(string $connection_string, int $connection_type = 0): ?resource;

function pg_pconnect(string $connection_string, int $connection_type = 0): ?resource;

function pg_async_connect(string $connection_string, int $connect_type = 0): ?resource;

function pg_connection_busy(resource $connection): bool;

function pg_connection_reset(resource $connection): bool;

function pg_connection_status(resource $connection): int;

function pg_convert(resource $connection, string $table_name, array<mixed> $assoc_array, int $option): mixed;

function pg_copy_from(resource $connection, string $table_name, array<mixed> $rows, string $delimiter="\t", string $null_as="\n"): bool;

function pg_copy_to(resource $connection, string $table_name, string $delimiter="\t", string $null_as="\n"): mixed;

function pg_dbname(resource $connection): ?string;

function pg_end_copy(resource $connection): bool;

function pg_escape_bytea(resource $connection, string $data): string;

function pg_escape_identifier(resource $connection, string $data): string;

function pg_escape_literal(resource $connection, string $data): string;

function pg_escape_string(resource $connection, string $data): string;

function pg_execute(resource $connection, string $stmtname, array<mixed> $params): ?resource;

function pg_fetch_all_columns(resource $result, int $column=0): ?array<string,mixed>;

function pg_fetch_all(resource $result): ?array<int,array<string,mixed>>;

function pg_fetch_array(resource $result, ?int $row = null, int $result_type = 3): ?array;

function pg_fetch_assoc(resource $result, ?int $row = null): ?array<string,?string>;

function pg_fetch_result(resource $result, ?int $row = null, mixed $field = null): mixed;

function pg_fetch_row(resource $result, ?int $row = null): ?array<int,?string>;

function pg_field_is_null(resource $result, mixed $row, mixed $field = null): ?int;

function pg_field_name(resource $result, int $field_number): ?string;

function pg_field_num(resource $result, string $field_name): int;

function pg_field_prtlen(resource $result, mixed $row_number, mixed $field = null_variant): ?int;

function pg_field_size(resource $result, int $field_number): ?int;

function pg_field_table(resource $result, int $field_number, bool $oid_only = false): mixed;

function pg_field_type_oid(resource $result, int $field_number): ?int;

function pg_field_type(resource $result, int $field_number): ?string;

function pg_free_result(resource $result): bool;

function pg_get_notify(resource $connection, int $result_type = 3): mixed;

function pg_get_pid(resource $connection): int;

function pg_get_result(resource $connection): ?resource;

function pg_host(resource $connection): ?string;

function pg_last_error(resource $connection): ?string;

function pg_last_notice(resource $connection): ?string;

function pg_last_oid(resource $result): mixed;

function pg_meta_data(resource $connection, string $table_name): mixed;

function pg_num_fields(resource $result): int;

function pg_num_rows(resource $result): int;

function pg_options(resource $connection): ?string;

function pg_parameter_status(resource $connection, string $param_name): mixed;

function pg_pconnect(string $connection_string, int $connect_type = 0): mixed;

function pg_ping(resource $connection): bool;

function pg_port(resource $connection): mixed;

function pg_prepare(resource $connection, string $stmtname, string $query): ?resource;

function pg_put_line(resource $connection, string $data): bool;

function pg_query_params(resource $connection, string $query, array<mixed> $params): ?resource;

function pg_query(resource $connection, string $query): ?resource;

function pg_result_error_field(resource $result, int $fieldcode): ?string;

function pg_result_error(resource $result): ?string;

function pg_result_seek(resource $result, int $offset): bool;

function pg_result_status(resource $result, int $type = 1): mixed;

function pg_send_execute(resource $connection, string $stmtname, array<mixed> $params): bool;

function pg_send_prepare(resource $connection, string $stmtname, string $query = null): bool;

function pg_send_query_params(resource $connection, string $query, array<mixed> $params): bool;

function pg_send_query(resource $connection, string $query): bool;

function pg_set_client_encoding(resource $connection, string $encoding): int;

function pg_trace(string $pathname, string $mode, resource $connection): bool;

function pg_transaction_status(resource $connection): int;

function pg_unescape_bytea(string $data): string;

function pg_untrace(resource $connection): bool;

function pg_version(resource $connection): ?array;
