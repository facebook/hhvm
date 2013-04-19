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

Value* fh_get_defined_functions(Value* _rv) asm("_ZN4HPHP23f_get_defined_functionsEv");

bool fh_function_exists(Value* function_name, bool autoload) asm("_ZN4HPHP17f_function_existsERKNS_6StringEb");

bool fh_is_callable(TypedValue* v, bool syntax, TypedValue* name) asm("_ZN4HPHP13f_is_callableERKNS_7VariantEbRKNS_14VRefParamValueE");

TypedValue* fh_call_user_func(TypedValue* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP16f_call_user_funcEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_call_user_func_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP22f_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE");

Value* fh_call_user_func_array_async(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP28f_call_user_func_array_asyncERKNS_7VariantERKNS_5ArrayE");

Value* fh_call_user_func_async(Value* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP22f_call_user_func_asyncEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_check_user_func_async(TypedValue* _rv, TypedValue* handles, int timeout) asm("_ZN4HPHP23f_check_user_func_asyncERKNS_7VariantEi");

TypedValue* fh_end_user_func_async(TypedValue* _rv, Value* handle, int default_strategy, TypedValue* additional_strategies) asm("_ZN4HPHP21f_end_user_func_asyncERKNS_6ObjectEiRKNS_7VariantE");

Value* fh_call_user_func_serialized(Value* _rv, Value* input) asm("_ZN4HPHP27f_call_user_func_serializedERKNS_6StringE");

TypedValue* fh_call_user_func_array_rpc(TypedValue* _rv, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* params) asm("_ZN4HPHP26f_call_user_func_array_rpcERKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_call_user_func_rpc(TypedValue* _rv, int64_t _argc, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* _argv) asm("_ZN4HPHP20f_call_user_func_rpcEiRKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_forward_static_call_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP27f_forward_static_call_arrayERKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_forward_static_call(TypedValue* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP21f_forward_static_callEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_get_called_class(TypedValue* _rv) asm("_ZN4HPHP18f_get_called_classEv");

Value* fh_create_function(Value* _rv, Value* args, Value* code) asm("_ZN4HPHP17f_create_functionERKNS_6StringES2_");

TypedValue* fh_func_get_arg(TypedValue* _rv, int arg_num) asm("_ZN4HPHP14f_func_get_argEi");

TypedValue* fh_func_get_args(TypedValue* _rv) asm("_ZN4HPHP15f_func_get_argsEv");

long fh_func_num_args() asm("_ZN4HPHP15f_func_num_argsEv");

void fh_register_postsend_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_postsend_functionEiRKNS_7VariantERKNS_5ArrayE");

void fh_register_shutdown_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_shutdown_functionEiRKNS_7VariantERKNS_5ArrayE");

void fh_register_cleanup_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP27f_register_cleanup_functionEiRKNS_7VariantERKNS_5ArrayE");

bool fh_register_tick_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_register_tick_functionEiRKNS_7VariantERKNS_5ArrayE");

void fh_unregister_tick_function(TypedValue* function_name) asm("_ZN4HPHP26f_unregister_tick_functionERKNS_7VariantE");

} // namespace HPHP
