<?hh   /* -*- php -*- */

namespace {
const FB_UNSERIALIZE_NONSTRING_VALUE = 0;
const FB_UNSERIALIZE_UNEXPECTED_END = 0;
const FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE = 0;
const FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE = 0;
const FB_UNSERIALIZE_MAX_DEPTH_EXCEEDED = 0;

const FB_SERIALIZE_HACK_ARRAYS = 0;
const FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS = 0;
const FB_SERIALIZE_VARRAY_DARRAY = 0;

const FB_COMPACT_SERIALIZE_FORCE_PHP_ARRAYS = 0;

const XHPROF_FLAGS_NO_BUILTINS = 0;
const XHPROF_FLAGS_CPU = 0;
const XHPROF_FLAGS_MEMORY = 0;
const XHPROF_FLAGS_VTSC = 0;
const XHPROF_FLAGS_TRACE = 0;
const XHPROF_FLAGS_MEASURE_XHPROF_DISABLE = 0;
const XHPROF_FLAGS_MALLOC = 0;
const XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY = 0;

const SETPROFILE_FLAGS_ENTERS = 1;
const SETPROFILE_FLAGS_EXITS = 2;
const SETPROFILE_FLAGS_DEFAULT = 3;
const SETPROFILE_FLAGS_FRAME_PTRS = 4;
const SETPROFILE_FLAGS_CTORS = 8;
const SETPROFILE_FLAGS_RESUME_AWARE = 16;
/* This flag enables access to $this upon instance method entry in the
 * setprofile handler. It *may break* in the future. */
const SETPROFILE_FLAGS_THIS_OBJECT__MAY_BREAK = 32;

const int PREG_FB__PRIVATE__HSL_IMPL = (1 << 29);

<<__PHPStdLib>>
function fb_serialize($thing, int $options = 0)[];
<<__PHPStdLib>>
function fb_unserialize($thing, inout $success, int $options = 0);
<<__PHPStdLib>>
function fb_compact_serialize($thing, int $options = 0)[];
<<__PHPStdLib>>
function fb_compact_unserialize($thing, inout $success, inout $errcode);
<<__PHPStdLib>>
function fb_intercept(string $name, $handler, $data = null);
<<__PHPStdLib>>
function fb_intercept2(string $name, $handler);
<<__PHPStdLib>>
function fb_rename_function(string $orig_func_name, string $new_func_name);
<<__PHPStdLib>>
function fb_utf8ize(inout $input);
<<__PHPStdLib>>
function fb_utf8_strlen(string $input)[];
<<__PHPStdLib>>
function fb_utf8_substr(string $str, int $start, int $length = PHP_INT_MAX)[];
<<__PHPStdLib>>
function fb_get_code_coverage(bool $flush);
<<__PHPStdLib>>
function fb_enable_code_coverage();
<<__PHPStdLib>>
function fb_disable_code_coverage();
<<__PHPStdLib>>
function xhprof_enable(int $flags = 0, $args = null);
<<__PHPStdLib>>
function xhprof_disable();
<<__PHPStdLib>>
function xhprof_network_enable();
<<__PHPStdLib>>
function xhprof_network_disable();
<<__PHPStdLib>>
function xhprof_frame_begin(string $name);
<<__PHPStdLib>>
function xhprof_frame_end();
<<__PHPStdLib>>
function xhprof_run_trace($packedTrace, $flags);
<<__PHPStdLib>>
function xhprof_sample_enable();
<<__PHPStdLib>>
function xhprof_sample_disable();
<<__PHPStdLib>>
function fb_output_compression(bool $new_value);
<<__PHPStdLib>>
function fb_set_exit_callback($function);
<<__PHPStdLib>>
function fb_get_last_flush_size();
<<__PHPStdLib>>
function fb_setprofile($callback, int $flags = SETPROFILE_FLAGS_DEFAULT, vec<string> $functions = vec[]);
} // namespace

namespace HH {
<<__PHPStdLib>>
function disable_code_coverage_with_frequency();
<<__PHPStdLib>>
function non_crypto_md5_upper(string $str)[]: int;
<<__PHPStdLib>>
function non_crypto_md5_lower(string $str)[]: int;

/** Returns the overflow part of multiplying two ints, as if they were unsigned.
 * In other words, this returns the upper 64 bits of the full product of
 * (unsigned)$a and (unsigned)$b. (The lower 64 bits is just `$a * $b`
 * regardless of signed/unsigned).
 */
function int_mul_overflow(int $a, int $b): int;

/** Returns the overflow part of multiplying two ints plus another int, as if
 * they were all unsigned. Specifically, this returns the upper 64 bits of
 * full (unsigned)$a * (unsigned)$b + (unsigned)$bias. $bias can be used to
 * manipulate rounding of the result.
 */
function int_mul_add_overflow(int $a, int $b, int $bias): int;

function enable_function_coverage();

function collect_function_coverage();
} // HH namespace
