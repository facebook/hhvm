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

bool fh_openssl_csr_export_to_file(TypedValue* csr, Value* outfilename, bool notext) asm("_ZN4HPHP28f_openssl_csr_export_to_fileERKNS_7VariantERKNS_6StringEb");

bool fh_openssl_csr_export(TypedValue* csr, TypedValue* out, bool notext) asm("_ZN4HPHP20f_openssl_csr_exportERKNS_7VariantERKNS_14VRefParamValueEb");

TypedValue* fh_openssl_csr_get_public_key(TypedValue* _rv, TypedValue* csr) asm("_ZN4HPHP28f_openssl_csr_get_public_keyERKNS_7VariantE");

TypedValue* fh_openssl_csr_get_subject(TypedValue* _rv, TypedValue* csr, bool use_shortnames) asm("_ZN4HPHP25f_openssl_csr_get_subjectERKNS_7VariantEb");

TypedValue* fh_openssl_csr_new(TypedValue* _rv, Value* dn, TypedValue* privkey, TypedValue* configargs, TypedValue* extraattribs) asm("_ZN4HPHP17f_openssl_csr_newERKNS_5ArrayERKNS_14VRefParamValueERKNS_7VariantES8_");

TypedValue* fh_openssl_csr_sign(TypedValue* _rv, TypedValue* csr, TypedValue* cacert, TypedValue* priv_key, int days, TypedValue* configargs, int serial) asm("_ZN4HPHP18f_openssl_csr_signERKNS_7VariantES2_S2_iS2_i");

TypedValue* fh_openssl_error_string(TypedValue* _rv) asm("_ZN4HPHP22f_openssl_error_stringEv");

void fh_openssl_free_key(Value* key) asm("_ZN4HPHP18f_openssl_free_keyERKNS_6ObjectE");

void fh_openssl_pkey_free(Value* key) asm("_ZN4HPHP19f_openssl_pkey_freeERKNS_6ObjectE");

bool fh_openssl_open(Value* sealed_data, TypedValue* open_data, Value* env_key, TypedValue* priv_key_id) asm("_ZN4HPHP14f_openssl_openERKNS_6StringERKNS_14VRefParamValueES2_RKNS_7VariantE");

bool fh_openssl_pkcs12_export_to_file(TypedValue* x509, Value* filename, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP31f_openssl_pkcs12_export_to_fileERKNS_7VariantERKNS_6StringES2_S5_S2_");

bool fh_openssl_pkcs12_export(TypedValue* x509, TypedValue* out, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP23f_openssl_pkcs12_exportERKNS_7VariantERKNS_14VRefParamValueES2_RKNS_6StringES2_");

bool fh_openssl_pkcs12_read(Value* pkcs12, TypedValue* certs, Value* pass) asm("_ZN4HPHP21f_openssl_pkcs12_readERKNS_6StringERKNS_14VRefParamValueES2_");

bool fh_openssl_pkcs7_decrypt(Value* infilename, Value* outfilename, TypedValue* recipcert, TypedValue* recipkey) asm("_ZN4HPHP23f_openssl_pkcs7_decryptERKNS_6StringES2_RKNS_7VariantES5_");

bool fh_openssl_pkcs7_encrypt(Value* infilename, Value* outfilename, TypedValue* recipcerts, Value* headers, int flags, int cipherid) asm("_ZN4HPHP23f_openssl_pkcs7_encryptERKNS_6StringES2_RKNS_7VariantERKNS_5ArrayEii");

bool fh_openssl_pkcs7_sign(Value* infilename, Value* outfilename, TypedValue* signcert, TypedValue* privkey, TypedValue* headers, int flags, Value* extracerts) asm("_ZN4HPHP20f_openssl_pkcs7_signERKNS_6StringES2_RKNS_7VariantES5_S5_iS2_");

TypedValue* fh_openssl_pkcs7_verify(TypedValue* _rv, Value* filename, int flags, Value* outfilename, Value* cainfo, Value* extracerts, Value* content) asm("_ZN4HPHP22f_openssl_pkcs7_verifyERKNS_6StringEiS2_RKNS_5ArrayES2_S2_");

bool fh_openssl_pkey_export_to_file(TypedValue* key, Value* outfilename, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP29f_openssl_pkey_export_to_fileERKNS_7VariantERKNS_6StringES5_S2_");

bool fh_openssl_pkey_export(TypedValue* key, TypedValue* out, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP21f_openssl_pkey_exportERKNS_7VariantERKNS_14VRefParamValueERKNS_6StringES2_");

Value* fh_openssl_pkey_get_details(Value* _rv, Value* key) asm("_ZN4HPHP26f_openssl_pkey_get_detailsERKNS_6ObjectE");

TypedValue* fh_openssl_pkey_get_private(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP26f_openssl_pkey_get_privateERKNS_7VariantERKNS_6StringE");

TypedValue* fh_openssl_get_privatekey(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP24f_openssl_get_privatekeyERKNS_7VariantERKNS_6StringE");

TypedValue* fh_openssl_pkey_get_public(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP25f_openssl_pkey_get_publicERKNS_7VariantE");

TypedValue* fh_openssl_get_publickey(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP23f_openssl_get_publickeyERKNS_7VariantE");

Value* fh_openssl_pkey_new(Value* _rv, TypedValue* configargs) asm("_ZN4HPHP18f_openssl_pkey_newERKNS_7VariantE");

bool fh_openssl_private_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

bool fh_openssl_private_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

bool fh_openssl_public_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

bool fh_openssl_public_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

TypedValue* fh_openssl_seal(TypedValue* _rv, Value* data, TypedValue* sealed_data, TypedValue* env_keys, Value* pub_key_ids) asm("_ZN4HPHP14f_openssl_sealERKNS_6StringERKNS_14VRefParamValueES5_RKNS_5ArrayE");

bool fh_openssl_sign(Value* data, TypedValue* signature, TypedValue* priv_key_id, int signature_alg) asm("_ZN4HPHP14f_openssl_signERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

TypedValue* fh_openssl_verify(TypedValue* _rv, Value* data, Value* signature, TypedValue* pub_key_id, int signature_alg) asm("_ZN4HPHP16f_openssl_verifyERKNS_6StringES2_RKNS_7VariantEi");

bool fh_openssl_x509_check_private_key(TypedValue* cert, TypedValue* key) asm("_ZN4HPHP32f_openssl_x509_check_private_keyERKNS_7VariantES2_");

long fh_openssl_x509_checkpurpose(TypedValue* x509cert, int purpose, Value* cainfo, Value* untrustedfile) asm("_ZN4HPHP27f_openssl_x509_checkpurposeERKNS_7VariantEiRKNS_5ArrayERKNS_6StringE");

bool fh_openssl_x509_export_to_file(TypedValue* x509, Value* outfilename, bool notext) asm("_ZN4HPHP29f_openssl_x509_export_to_fileERKNS_7VariantERKNS_6StringEb");

bool fh_openssl_x509_export(TypedValue* x509, TypedValue* output, bool notext) asm("_ZN4HPHP21f_openssl_x509_exportERKNS_7VariantERKNS_14VRefParamValueEb");

void fh_openssl_x509_free(Value* x509cert) asm("_ZN4HPHP19f_openssl_x509_freeERKNS_6ObjectE");

TypedValue* fh_openssl_x509_parse(TypedValue* _rv, TypedValue* x509cert, bool shortnames) asm("_ZN4HPHP20f_openssl_x509_parseERKNS_7VariantEb");

TypedValue* fh_openssl_x509_read(TypedValue* _rv, TypedValue* x509certdata) asm("_ZN4HPHP19f_openssl_x509_readERKNS_7VariantE");

TypedValue* fh_openssl_random_pseudo_bytes(TypedValue* _rv, int length, TypedValue* crypto_strong) asm("_ZN4HPHP29f_openssl_random_pseudo_bytesEiRKNS_14VRefParamValueE");

TypedValue* fh_openssl_cipher_iv_length(TypedValue* _rv, Value* method) asm("_ZN4HPHP26f_openssl_cipher_iv_lengthERKNS_6StringE");

TypedValue* fh_openssl_encrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_encryptERKNS_6StringES2_S2_iS2_");

TypedValue* fh_openssl_decrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_decryptERKNS_6StringES2_S2_iS2_");

TypedValue* fh_openssl_digest(TypedValue* _rv, Value* data, Value* method, bool raw_output) asm("_ZN4HPHP16f_openssl_digestERKNS_6StringES2_b");

Value* fh_openssl_get_cipher_methods(Value* _rv, bool aliases) asm("_ZN4HPHP28f_openssl_get_cipher_methodsEb");

Value* fh_openssl_get_md_methods(Value* _rv, bool aliases) asm("_ZN4HPHP24f_openssl_get_md_methodsEb");

} // namespace HPHP
