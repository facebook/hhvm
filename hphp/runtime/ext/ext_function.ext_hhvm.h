/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
namespace HPHP {

/*
HPHP::Array HPHP::f_get_defined_functions()
_ZN4HPHP23f_get_defined_functionsEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_defined_functions(Value* _rv) asm("_ZN4HPHP23f_get_defined_functionsEv");

/*
bool HPHP::f_function_exists(HPHP::String const&, bool)
_ZN4HPHP17f_function_existsERKNS_6StringEb

(return value) => rax
function_name => rdi
autoload => rsi
*/

bool fh_function_exists(Value* function_name, bool autoload) asm("_ZN4HPHP17f_function_existsERKNS_6StringEb");

/*
bool HPHP::f_is_callable(HPHP::Variant const&, bool, HPHP::VRefParamValue const&)
_ZN4HPHP13f_is_callableERKNS_7VariantEbRKNS_14VRefParamValueE

(return value) => rax
v => rdi
syntax => rsi
name => rdx
*/

bool fh_is_callable(TypedValue* v, bool syntax, TypedValue* name) asm("_ZN4HPHP13f_is_callableERKNS_7VariantEbRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_call_user_func(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP16f_call_user_funcEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
_argv => rcx
*/

TypedValue* fh_call_user_func(TypedValue* _rv, long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP16f_call_user_funcEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Object HPHP::f_call_user_func_array_async(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP28f_call_user_func_array_asyncERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

Value* fh_call_user_func_array_async(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP28f_call_user_func_array_asyncERKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Object HPHP::f_call_user_func_async(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP22f_call_user_func_asyncEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
_argv => rcx
*/

Value* fh_call_user_func_async(Value* _rv, long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP22f_call_user_func_asyncEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_check_user_func_async(HPHP::Variant const&, int)
_ZN4HPHP23f_check_user_func_asyncERKNS_7VariantEi

(return value) => rax
_rv => rdi
handles => rsi
timeout => rdx
*/

TypedValue* fh_check_user_func_async(TypedValue* _rv, TypedValue* handles, int timeout) asm("_ZN4HPHP23f_check_user_func_asyncERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_end_user_func_async(HPHP::Object const&, int, HPHP::Variant const&)
_ZN4HPHP21f_end_user_func_asyncERKNS_6ObjectEiRKNS_7VariantE

(return value) => rax
_rv => rdi
handle => rsi
default_strategy => rdx
additional_strategies => rcx
*/

TypedValue* fh_end_user_func_async(TypedValue* _rv, Value* handle, int default_strategy, TypedValue* additional_strategies) asm("_ZN4HPHP21f_end_user_func_asyncERKNS_6ObjectEiRKNS_7VariantE");

/*
HPHP::String HPHP::f_call_user_func_serialized(HPHP::String const&)
_ZN4HPHP27f_call_user_func_serializedERKNS_6StringE

(return value) => rax
_rv => rdi
input => rsi
*/

Value* fh_call_user_func_serialized(Value* _rv, Value* input) asm("_ZN4HPHP27f_call_user_func_serializedERKNS_6StringE");

/*
HPHP::Variant HPHP::f_call_user_func_array_rpc(HPHP::String const&, int, HPHP::String const&, int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP26f_call_user_func_array_rpcERKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
host => rsi
port => rdx
auth => rcx
timeout => r8
function => r9
params => st0
*/

TypedValue* fh_call_user_func_array_rpc(TypedValue* _rv, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* params) asm("_ZN4HPHP26f_call_user_func_array_rpcERKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_call_user_func_rpc(int, HPHP::String const&, int, HPHP::String const&, int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP20f_call_user_func_rpcEiRKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
host => rdx
port => rcx
auth => r8
timeout => r9
function => st0
_argv => st8
*/

TypedValue* fh_call_user_func_rpc(TypedValue* _rv, long long _argc, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* _argv) asm("_ZN4HPHP20f_call_user_func_rpcEiRKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_forward_static_call_array(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP27f_forward_static_call_arrayERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

TypedValue* fh_forward_static_call_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP27f_forward_static_call_arrayERKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_forward_static_call(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP21f_forward_static_callEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
_argv => rcx
*/

TypedValue* fh_forward_static_call(TypedValue* _rv, long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP21f_forward_static_callEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_get_called_class()
_ZN4HPHP18f_get_called_classEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_get_called_class(TypedValue* _rv) asm("_ZN4HPHP18f_get_called_classEv");

/*
HPHP::String HPHP::f_create_function(HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_create_functionERKNS_6StringES2_

(return value) => rax
_rv => rdi
args => rsi
code => rdx
*/

Value* fh_create_function(Value* _rv, Value* args, Value* code) asm("_ZN4HPHP17f_create_functionERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_func_get_arg(int)
_ZN4HPHP14f_func_get_argEi

(return value) => rax
_rv => rdi
arg_num => rsi
*/

TypedValue* fh_func_get_arg(TypedValue* _rv, int arg_num) asm("_ZN4HPHP14f_func_get_argEi");

/*
HPHP::Array HPHP::f_func_get_args()
_ZN4HPHP15f_func_get_argsEv

(return value) => rax
_rv => rdi
*/

Value* fh_func_get_args(Value* _rv) asm("_ZN4HPHP15f_func_get_argsEv");

/*
long long HPHP::f_func_num_args()
_ZN4HPHP15f_func_num_argsEv

(return value) => rax
*/

long long fh_func_num_args() asm("_ZN4HPHP15f_func_num_argsEv");

/*
void HPHP::f_register_postsend_function(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP28f_register_postsend_functionEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
function => rsi
_argv => rdx
*/

void fh_register_postsend_function(long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_postsend_functionEiRKNS_7VariantERKNS_5ArrayE");

/*
void HPHP::f_register_shutdown_function(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP28f_register_shutdown_functionEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
function => rsi
_argv => rdx
*/

void fh_register_shutdown_function(long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_shutdown_functionEiRKNS_7VariantERKNS_5ArrayE");

/*
void HPHP::f_register_cleanup_function(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP27f_register_cleanup_functionEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
function => rsi
_argv => rdx
*/

void fh_register_cleanup_function(long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP27f_register_cleanup_functionEiRKNS_7VariantERKNS_5ArrayE");

/*
bool HPHP::f_register_tick_function(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_register_tick_functionEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_argc => rdi
function => rsi
_argv => rdx
*/

bool fh_register_tick_function(long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_register_tick_functionEiRKNS_7VariantERKNS_5ArrayE");

/*
void HPHP::f_unregister_tick_function(HPHP::Variant const&)
_ZN4HPHP26f_unregister_tick_functionERKNS_7VariantE

function_name => rdi
*/

void fh_unregister_tick_function(TypedValue* function_name) asm("_ZN4HPHP26f_unregister_tick_functionERKNS_7VariantE");


} // !HPHP

