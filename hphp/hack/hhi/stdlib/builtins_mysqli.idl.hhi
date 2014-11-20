<?hh     /* -*- php -*- */

define("MYSQLI_REFRESH_SLAVE", 64);
define("MYSQLI_BLOB_FLAG", 16);
define("MYSQLI_ENUM_FLAG", 256);
define("MYSQLI_TYPE_TIMESTAMP", 7);
define("MYSQLI_TYPE_DATETIME", 12);
define("MYSQLI_TYPE_LONGLONG", 8);
define("MYSQLI_OPT_CAN_HANDLE_EXPIRED_PASSWORDS", 37);
define("MYSQLI_TRANS_START_READ_ONLY", 4);
define("MYSQLI_REPORT_STRICT", 2);
define("MYSQLI_SERVER_QUERY_WAS_SLOW", 2048);
define("MYSQLI_TRANS_COR_AND_CHAIN", 1);
define("MYSQLI_TIMESTAMP_FLAG", 1024);
define("MYSQLI_PRI_KEY_FLAG", 2);
define("MYSQLI_TYPE_CHAR", 1);
define("MYSQLI_SERVER_PUBLIC_KEY", 35);
define("MYSQLI_AUTO_INCREMENT_FLAG", 512);
define("MYSQLI_REFRESH_HOSTS", 8);
define("MYSQLI_INIT_COMMAND", 3);
define("MYSQLI_TYPE_TINY", 1);
define("MYSQLI_TYPE_INT24", 9);
define("MYSQLI_ASSOC", 1);
define("MYSQLI_TYPE_TIME", 11);
define("MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH", 0);
define("MYSQLI_TRANS_COR_NO_RELEASE", 8);
define("MYSQLI_REFRESH_TABLES", 4);
define("MYSQLI_UNIQUE_KEY_FLAG", 4);
define("MYSQLI_REFRESH_GRANT", 1);
define("MYSQLI_TRANS_COR_RELEASE", 4);
define("MYSQLI_CURSOR_TYPE_SCROLLABLE", 4);
define("MYSQLI_TRANS_START_READ_WRITE", 2);
define("MYSQLI_TYPE_BIT", 16);
define("MYSQLI_SERVER_QUERY_NO_INDEX_USED", 32);
define("MYSQLI_TYPE_LONG", 3);
define("MYSQLI_NUM_FLAG", 32768);
define("MYSQLI_OPT_WRITE_TIMEOUT", 12);
define("MYSQLI_TYPE_MEDIUM_BLOB", 250);
define("MYSQLI_CLIENT_INTERACTIVE", 1024);
define("MYSQLI_TYPE_DECIMAL", 0);
define("MYSQLI_CURSOR_TYPE_FOR_UPDATE", 2);
define("MYSQLI_TYPE_VAR_STRING", 253);
define("MYSQLI_SET_CHARSET_NAME", 7);
define("MYSQLI_UNSIGNED_FLAG", 32);
define("MYSQLI_DATA_TRUNCATED", 101);
define("MYSQLI_TYPE_GEOMETRY", 255);
define("MYSQLI_TRANS_START_WITH_CONSISTENT_SNAPSHOT", 1);
define("MYSQLI_TYPE_NEWDATE", 14);
define("MYSQLI_NO_DATA", 100);
define("MYSQLI_SECURE_AUTH", 18);
define("MYSQLI_REPORT_ALL", 255);
define("MYSQLI_SHARED_MEMORY_BASE_NAME", 10);
define("MYSQLI_TYPE_DOUBLE", 5);
define("MYSQLI_TYPE_INTERVAL", 247);
define("MYSQLI_REPORT_OFF", 0);
define("MYSQLI_NO_DEFAULT_VALUE_FLAG", 4096);
define("MYSQLI_REFRESH_MASTER", 128);
define("MYSQLI_TYPE_DATE", 10);
define("MYSQLI_GROUP_FLAG", 32768);
define("MYSQLI_REFRESH_THREADS", 32);
define("MYSQLI_CLIENT_IGNORE_SPACE", 256);
define("MYSQLI_TYPE_STRING", 254);
define("MYSQLI_TYPE_BLOB", 252);
define("MYSQLI_STMT_ATTR_PREFETCH_ROWS", 2);
define("MYSQLI_TYPE_YEAR", 13);
define("MYSQLI_REFRESH_LOG", 2);
define("MYSQLI_OPT_USE_REMOTE_CONNECTION", 14);
define("MYSQLI_TRANS_COR_AND_NO_CHAIN", 2);
define("MYSQLI_CURSOR_TYPE_NO_CURSOR", 0);
define("MYSQLI_OPT_COMPRESS", 1);
define("MYSQLI_TYPE_SET", 248);
define("MYSQLI_OPT_SSL_VERIFY_SERVER_CERT", 21);
define("MYSQLI_SET_CLIENT_IP", 17);
define("MYSQLI_OPT_NAMED_PIPE", 2);
define("MYSQLI_OPT_LOCAL_INFILE", 8);
define("MYSQLI_MULTIPLE_KEY_FLAG", 8);
define("MYSQLI_CLIENT_SSL", 2048);
define("MYSQLI_CURSOR_TYPE_READ_ONLY", 1);
define("MYSQLI_OPT_READ_TIMEOUT", 11);
define("MYSQLI_SERVER_QUERY_NO_GOOD_INDEX_USED", 16);
define("MYSQLI_OPT_PROTOCOL", 9);
define("MYSQLI_REFRESH_STATUS", 16);
define("MYSQLI_CLIENT_NO_SCHEMA", 16);
define("MYSQLI_BINARY_FLAG", 128);
define("MYSQLI_TYPE_ENUM", 247);
define("MYSQLI_TYPE_VARCHAR", 15);
define("MYSQLI_REPORT_CLOSE", 8);
define("MYSQLI_REPORT_INDEX", 4);
define("MYSQLI_PART_KEY_FLAG", 16384);
define("MYSQLI_NUM", 2);
define("MYSQLI_SET_CHARSET_DIR", 6);
define("MYSQLI_STMT_ATTR_CURSOR_TYPE", 1);
define("MYSQLI_ZEROFILL_FLAG", 64);
define("MYSQLI_TYPE_SHORT", 2);
define("MYSQLI_CLIENT_COMPRESS", 32);
define("MYSQLI_NOT_NULL_FLAG", 1);
define("MYSQLI_OPT_RECONNECT", 20);
define("MYSQLI_CLIENT_FOUND_ROWS", 2);
define("MYSQLI_READ_DEFAULT_GROUP", 5);
define("MYSQLI_STORE_RESULT", 0);
define("MYSQLI_OPT_CONNECT_TIMEOUT", 0);
define("MYSQLI_SET_FLAG", 2048);
define("MYSQLI_OPT_USE_EMBEDDED_CONNECTION", 15);
define("MYSQLI_REPORT_DATA_TRUNCATION", 19);
define("MYSQLI_TYPE_NULL", 6);
define("MYSQLI_TYPE_TINY_BLOB", 249);
define("MYSQLI_USE_RESULT", 1);
define("MYSQLI_TYPE_NEWDECIMAL", 246);
define("MYSQLI_DEBUG_TRACE_ENABLED", 0);
define("MYSQLI_READ_DEFAULT_FILE", 4);
define("MYSQLI_BOTH", 3);
define("MYSQLI_TYPE_FLOAT", 4);
define("MYSQLI_TYPE_LONG_BLOB", 251);
define("MYSQLI_OPT_GUESS_CONNECTION", 16);
define("MYSQLI_REPORT_ERROR", 1);

class mysqli {

	public static $__connection_errno = 0;
	public static $__connection_error = null;

	/*
	 * Dynamic __get() properties
	 */
	public $connect_errno;
	public $connect_error;
	public $client_info;
	public $client_version;
	public $affected_rows;
	public $error;
	public $errno;
	public $field_count;
	public $host_info;
	public $info;
	public $insert_id;
	public $protocol_version;
	public $server_info;
	public $server_version;
	public $sqlstate;
	public $thread_id;
	public $warning_count;
	public $error_list;

	public function __get($name) {}
	public function __clone() {}
	public function __construct($host = null, $username = null, $passwd = null, $dbname = null, $port = null, $socket = null) {}

	public function connect($host = null, $username = null, $passwd = null, $dbname = null, $port = null, $socket = null) {}
	public function init() {}
	public function stmt_init() {}
	public function close() {}
	public function commit($flags = 0, $name = null) {}
	public function escape_string($escapestr) {}
	public function get_client_info() {}
	public function get_warnings() {}
	public function more_results() {}
	public function multi_query($query) {}
	public function next_result() {}
	public function ping() {}
	public function prepare($query) {}
	public function query($query, $resultmode = MYSQLI_STORE_RESULT) {}
	public function real_connect($host = null, $username = null, $passwd = null, $dbname = null, $port = null, $socket = null, $flags = 0) {}
	public function real_escape_string($escapestr) {}
	public function real_query($query) {}
	public function release_savepoint($name) {}
	public function rollback($flags = 0, $name = null) {}
	public function savepoint($name) {}
	public function select_db($dbname) {}
	public function set_charset($charset) {}
	public function set_opt($option, $value) {}
	public function ssl_set($key, $cert, $ca, $capath, $cipher) {}
	public function stat() {}
	public function store_result() {}
	public function use_result() {}
	public function autocommit($mode) {}
	public function begin_transaction($flags = null, $name = null) {}
	public function change_user($user, $password, $database) {}
	public function character_set_name() {}
	public function dump_debug_info() {}
	public function get_charset() {}
	public function kill($processid) {}
	public function options($option, $value) {}
	public function refresh(int $options) {}
}

class mysqli_driver {

	/*
	 * Dynamic __get() properties
	 */
	public $client_info;
	public $client_version;
	public $driver_version;
	public $embedded;
	public $reconnect;
	public $report_mode;

	public function __get($name) {}
	public function __set($name, $value) {}
	public function __clone() {}
}

class mysqli_result {

	/*
	 * Dynamic __get() properties
	 */
	public $current_field;
	public $field_count;
	public $lengths;
	public $num_rows;

	public function __construct($result, $resulttype = MYSQLI_STORE_RESULT) {}
	public function __clone() {}
	public function __get($name) {}
	
	public function close() {}
	public function data_seek($offset) {}
	public function fetch_all($resulttype = MYSQLI_NUM) {}
	public function fetch_array($resulttype = MYSQLI_BOTH) {}
	public function fetch_assoc() {}
	public function fetch_field_direct($fieldnr) {}
	public function fetch_field() {}
	public function fetch_fields() {}
	public function fetch_object($class_name = null, $params = array()) {}
	public function fetch_row() {}
	public function field_seek($fieldnr) {}
	public function free() {}
	public function free_result() {}
}

class mysqli_stmt {
	/*
	 * Dynamic __get() properties
	 */
	public $affected_rows;
	public $errno;
	public $error_list;
	public $erro;
	public $field_count;
	public $insert_id;
	public $num_rows;
	public $param_count;
	public $sqlstate;

	public function __construct($link, $query = null) {}
	public function __clone() {}
	public function __get(string $name) {}

	public function attr_get($attr) {}
	public function attr_set($attr, $mode) {}
	public function bind_param(string $types) {}
	public function bind_result() {}
	public function close() {}
	public function data_seek($offset) {}
	public function execute() {}
	public function fetch() {}
	public function free_result() {}
	public function get_warnings() {}
	public function prepare($query) {}
	public function reset() {}
	public function result_metadata() {}
	public function send_long_data($param_nr, $data) {}
	public function store_result() {}
	public function hh_store_result() {}
}

class mysqli_warning {

	public $message;
	public $sqlstate = "HY000";
	public int $errno;

	public function __construct($warnings) {}
	public function next() {}
}

function mysqli_affected_rows($link) {}
function mysqli_autocommit($link, $mode) {}
function mysqli_begin_transaction($link, $flags, $name) {}
function mysqli_connect($host = null, $username = null, $passwd = null, $dbname = null, $port = null, $socket = null) {}
function mysqli_change_user($link, string $user, $password, $database) {}
function mysqli_character_set_name($link) {}
function mysqli_get_client_info() {}
function mysqli_get_client_version() {}
function mysqli_close($link) {}
function mysqli_commit($link, $flags = 0, $name = null) {}
function mysqli_connect_errno() {}
function mysqli_connect_error() {}
function mysqli_dump_debug_info($link) {}
function mysqli_errno($link) {}
function mysqli_error_list($link) {}
function mysqli_error($link) {}
function mysqli_field_count($link) {}
function mysqli_get_charset($link) {}
function mysqli_get_host_info($link) {}
function mysqli_get_proto_info($link) {}
function mysqli_get_server_info($link) {}
function mysqli_get_server_version($link) {}
function mysqli_get_warnings($link) {}
function mysqli_info($link) {}
function mysqli_init() {}
function mysqli_insert_id($link) {}
function mysqli_kill($link, $processid) {}
function mysqli_more_results($link) {}
function mysqli_multi_query($link, $query) {}
function mysqli_next_result($link) {}
function mysqli_options($link, $option, $value) {}
function mysqli_ping($link) {}
function mysqli_prepare($link, $query) {}
function mysqli_query($link, $query, $resultmode = MYSQLI_STORE_RESULT) {}
function mysqli_real_connect($link, $host = null, $username = null, $passwd = null, $dbname = null, $port = null, $socket = null, $flags = 0) {}
function mysqli_escape_string($link, $escapestr) {}
function mysqli_real_escape_string($link, $escapestr) {}
function mysqli_real_query($link, $query) {}
function mysqli_refresh($link, $options) {}
function mysqli_release_savepoint($link, $name) {}
function mysqli_rollback($link, $flags = 0, $name = null) {}
function mysqli_savepoint($link, $name) {}
function mysqli_select_db($link, $dbname) {}
function mysqli_set_charset($link, $charset) {}
function mysqli_sqlstate($link) {}
function mysqli_ssl_set($link, $key, $cert, $ca, $capath, $cipher) {}
function mysqli_stat($link) {}
function mysqli_stmt_init($link) {}
function mysqli_store_result($link) {}
function mysqli_thread_id($link) {}
function mysqli_thread_safe() {}
function mysqli_use_result($link) {}
function mysqli_warning_count($link) {}
function mysqli_report($flags) {}
function mysqli_set_opt($link, $option, $value) {}
function mysqli_field_tell($result) {}
function mysqli_data_seek($result, $offset) {}
function mysqli_fetch_all($result, $resulttype = MYSQLI_NUM) {}
function mysqli_fetch_array($result, $resulttype = MYSQLI_BOTH) {}
function mysqli_fetch_assoc($result) {}
function mysqli_fetch_field_direct($result, $fieldnr) {}
function mysqli_fetch_field($result) {}
function mysqli_fetch_fields($result) {}
function mysqli_fetch_object($result, $class_name = null, $params = array()) {}
function mysqli_fetch_row($result) {}
function mysqli_num_fields($result) {}
function mysqli_field_seek($result, $fieldnr) {}
function mysqli_free_result($result) {}
function mysqli_fetch_lengths($result) {}
function mysqli_num_rows($result) {}
function mysqli_stmt_affected_rows($stmt) {}
function mysqli_stmt_attr_get($stmt, $attr) {}
function mysqli_stmt_attr_set($stmt, $attr, $mode) {}
function mysqli_stmt_bind_param($stmt, $types) {}
function mysqli_stmt_bind_result($stmt) {}
function mysqli_stmt_close($stmt) {}
function mysqli_stmt_data_seek($stmt, $offset) {}
function mysqli_stmt_errno($stmt) {}
function mysqli_stmt_error_list($stmt) {}
function mysqli_stmt_error($stmt) {}
function mysqli_stmt_execute($stmt) {}
function mysqli_stmt_fetch($stmt) {}
function mysqli_stmt_field_count($stmt) {}
function mysqli_stmt_free_result($stmt) {}
function mysqli_stmt_get_warnings($stmt) {}
function mysqli_stmt_insert_id($stmt) {}
function mysqli_stmt_more_results($stmt) {}
function mysqli_stmt_next_result($stmt) {}
function mysqli_stmt_num_rows($stmt) {}
function mysqli_stmt_param_count($stmt) {}
function mysqli_stmt_prepare($stmt, $query) {}
function mysqli_stmt_reset($stmt) {}
function mysqli_stmt_result_metadata($stmt) {}
function mysqli_stmt_send_long_data($stmt, $param_nr, $data) {}
function mysqli_stmt_sqlstate($stmt) {}
function mysqli_stmt_store_result($stmt) {}
