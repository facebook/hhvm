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
HPHP::Variant HPHP::f_mcrypt_module_open(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_

(return value) => rax
_rv => rdi
algorithm => rsi
algorithm_directory => rdx
mode => rcx
mode_directory => r8
*/

TypedValue* fh_mcrypt_module_open(TypedValue* _rv, Value* algorithm, Value* algorithm_directory, Value* mode, Value* mode_directory) asm("_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_");

/*
bool HPHP::f_mcrypt_module_close(HPHP::Object const&)
_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_module_close(Value* td) asm("_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_mcrypt_list_algorithms(HPHP::String const&)
_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE

(return value) => rax
_rv => rdi
lib_dir => rsi
*/

Value* fh_mcrypt_list_algorithms(Value* _rv, Value* lib_dir) asm("_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE");

/*
HPHP::Array HPHP::f_mcrypt_list_modes(HPHP::String const&)
_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE

(return value) => rax
_rv => rdi
lib_dir => rsi
*/

Value* fh_mcrypt_list_modes(Value* _rv, Value* lib_dir) asm("_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE");

/*
long long HPHP::f_mcrypt_module_get_algo_block_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

long long fh_mcrypt_module_get_algo_block_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_");

/*
long long HPHP::f_mcrypt_module_get_algo_key_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

long long fh_mcrypt_module_get_algo_key_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_mcrypt_module_get_supported_key_sizes(HPHP::String const&, HPHP::String const&)
_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_

(return value) => rax
_rv => rdi
algorithm => rsi
lib_dir => rdx
*/

Value* fh_mcrypt_module_get_supported_key_sizes(Value* _rv, Value* algorithm, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_");

/*
bool HPHP::f_mcrypt_module_is_block_algorithm_mode(HPHP::String const&, HPHP::String const&)
_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_

(return value) => rax
mode => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_algorithm_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_");

/*
bool HPHP::f_mcrypt_module_is_block_algorithm(HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_algorithm(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_");

/*
bool HPHP::f_mcrypt_module_is_block_mode(HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_

(return value) => rax
mode => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_");

/*
bool HPHP::f_mcrypt_module_self_test(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_self_test(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mcrypt_create_iv(int, int)
_ZN4HPHP18f_mcrypt_create_ivEii

(return value) => rax
_rv => rdi
size => rsi
source => rdx
*/

TypedValue* fh_mcrypt_create_iv(TypedValue* _rv, int size, int source) asm("_ZN4HPHP18f_mcrypt_create_ivEii");

/*
HPHP::Variant HPHP::f_mcrypt_encrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_encrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_");

/*
HPHP::Variant HPHP::f_mcrypt_decrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_decrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_");

/*
HPHP::Variant HPHP::f_mcrypt_cbc(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_cbc(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_mcrypt_cfb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_cfb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_mcrypt_ecb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_ecb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_mcrypt_ofb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_ofb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_mcrypt_get_block_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_

(return value) => rax
_rv => rdi
cipher => rsi
module => rdx
*/

TypedValue* fh_mcrypt_get_block_size(TypedValue* _rv, Value* cipher, Value* module) asm("_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mcrypt_get_cipher_name(HPHP::String const&)
_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE

(return value) => rax
_rv => rdi
cipher => rsi
*/

TypedValue* fh_mcrypt_get_cipher_name(TypedValue* _rv, Value* cipher) asm("_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mcrypt_get_iv_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_

(return value) => rax
_rv => rdi
cipher => rsi
mode => rdx
*/

TypedValue* fh_mcrypt_get_iv_size(TypedValue* _rv, Value* cipher, Value* mode) asm("_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_");

/*
long long HPHP::f_mcrypt_get_key_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_

(return value) => rax
cipher => rdi
module => rsi
*/

long long fh_mcrypt_get_key_size(Value* cipher, Value* module) asm("_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_");

/*
HPHP::String HPHP::f_mcrypt_enc_get_algorithms_name(HPHP::Object const&)
_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_algorithms_name(Value* _rv, Value* td) asm("_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE");

/*
long long HPHP::f_mcrypt_enc_get_block_size(HPHP::Object const&)
_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long long fh_mcrypt_enc_get_block_size(Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE");

/*
long long HPHP::f_mcrypt_enc_get_iv_size(HPHP::Object const&)
_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long long fh_mcrypt_enc_get_iv_size(Value* td) asm("_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE");

/*
long long HPHP::f_mcrypt_enc_get_key_size(HPHP::Object const&)
_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long long fh_mcrypt_enc_get_key_size(Value* td) asm("_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_mcrypt_enc_get_modes_name(HPHP::Object const&)
_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_modes_name(Value* _rv, Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_mcrypt_enc_get_supported_key_sizes(HPHP::Object const&)
_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_supported_key_sizes(Value* _rv, Value* td) asm("_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE");

/*
bool HPHP::f_mcrypt_enc_is_block_algorithm_mode(HPHP::Object const&)
_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_algorithm_mode(Value* td) asm("_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE");

/*
bool HPHP::f_mcrypt_enc_is_block_algorithm(HPHP::Object const&)
_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_algorithm(Value* td) asm("_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE");

/*
bool HPHP::f_mcrypt_enc_is_block_mode(HPHP::Object const&)
_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_mode(Value* td) asm("_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE");

/*
long long HPHP::f_mcrypt_enc_self_test(HPHP::Object const&)
_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long long fh_mcrypt_enc_self_test(Value* td) asm("_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_mcrypt_generic(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
td => rsi
data => rdx
*/

TypedValue* fh_mcrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE");

/*
long long HPHP::f_mcrypt_generic_init(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
td => rdi
key => rsi
iv => rdx
*/

long long fh_mcrypt_generic_init(Value* td, Value* key, Value* iv) asm("_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_mdecrypt_generic(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
td => rsi
data => rdx
*/

TypedValue* fh_mdecrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_mcrypt_generic_deinit(HPHP::Object const&)
_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_generic_deinit(Value* td) asm("_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE");

/*
bool HPHP::f_mcrypt_generic_end(HPHP::Object const&)
_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_generic_end(Value* td) asm("_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE");


} // !HPHP

