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

TypedValue* fh_mcrypt_module_open(TypedValue* _rv, Value* algorithm, Value* algorithm_directory, Value* mode, Value* mode_directory) asm("_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_");

bool fh_mcrypt_module_close(Value* td) asm("_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE");

Value* fh_mcrypt_list_algorithms(Value* _rv, Value* lib_dir) asm("_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE");

Value* fh_mcrypt_list_modes(Value* _rv, Value* lib_dir) asm("_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE");

long fh_mcrypt_module_get_algo_block_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_");

long fh_mcrypt_module_get_algo_key_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_");

Value* fh_mcrypt_module_get_supported_key_sizes(Value* _rv, Value* algorithm, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_");

bool fh_mcrypt_module_is_block_algorithm_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_");

bool fh_mcrypt_module_is_block_algorithm(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_");

bool fh_mcrypt_module_is_block_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_");

bool fh_mcrypt_module_self_test(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_");

TypedValue* fh_mcrypt_create_iv(TypedValue* _rv, int size, int source) asm("_ZN4HPHP18f_mcrypt_create_ivEii");

TypedValue* fh_mcrypt_encrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_");

TypedValue* fh_mcrypt_decrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_");

TypedValue* fh_mcrypt_cbc(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_");

TypedValue* fh_mcrypt_cfb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_");

TypedValue* fh_mcrypt_ecb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_");

TypedValue* fh_mcrypt_ofb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_");

TypedValue* fh_mcrypt_get_block_size(TypedValue* _rv, Value* cipher, Value* module) asm("_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_");

TypedValue* fh_mcrypt_get_cipher_name(TypedValue* _rv, Value* cipher) asm("_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE");

TypedValue* fh_mcrypt_get_iv_size(TypedValue* _rv, Value* cipher, Value* mode) asm("_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_");

long fh_mcrypt_get_key_size(Value* cipher, Value* module) asm("_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_");

Value* fh_mcrypt_enc_get_algorithms_name(Value* _rv, Value* td) asm("_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE");

long fh_mcrypt_enc_get_block_size(Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE");

long fh_mcrypt_enc_get_iv_size(Value* td) asm("_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE");

long fh_mcrypt_enc_get_key_size(Value* td) asm("_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE");

Value* fh_mcrypt_enc_get_modes_name(Value* _rv, Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE");

Value* fh_mcrypt_enc_get_supported_key_sizes(Value* _rv, Value* td) asm("_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE");

bool fh_mcrypt_enc_is_block_algorithm_mode(Value* td) asm("_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE");

bool fh_mcrypt_enc_is_block_algorithm(Value* td) asm("_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE");

bool fh_mcrypt_enc_is_block_mode(Value* td) asm("_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE");

long fh_mcrypt_enc_self_test(Value* td) asm("_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE");

long fh_mcrypt_generic_init(Value* td, Value* key, Value* iv) asm("_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_mcrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_mdecrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE");

bool fh_mcrypt_generic_deinit(Value* td) asm("_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE");

bool fh_mcrypt_generic_end(Value* td) asm("_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE");

} // namespace HPHP
