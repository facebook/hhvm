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
bool HPHP::f_openssl_csr_export_to_file(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP28f_openssl_csr_export_to_fileERKNS_7VariantERKNS_6StringEb

(return value) => rax
csr => rdi
outfilename => rsi
notext => rdx
*/

bool fh_openssl_csr_export_to_file(TypedValue* csr, Value* outfilename, bool notext) asm("_ZN4HPHP28f_openssl_csr_export_to_fileERKNS_7VariantERKNS_6StringEb");

/*
bool HPHP::f_openssl_csr_export(HPHP::Variant const&, HPHP::VRefParamValue const&, bool)
_ZN4HPHP20f_openssl_csr_exportERKNS_7VariantERKNS_14VRefParamValueEb

(return value) => rax
csr => rdi
out => rsi
notext => rdx
*/

bool fh_openssl_csr_export(TypedValue* csr, TypedValue* out, bool notext) asm("_ZN4HPHP20f_openssl_csr_exportERKNS_7VariantERKNS_14VRefParamValueEb");

/*
HPHP::Variant HPHP::f_openssl_csr_get_public_key(HPHP::Variant const&)
_ZN4HPHP28f_openssl_csr_get_public_keyERKNS_7VariantE

(return value) => rax
_rv => rdi
csr => rsi
*/

TypedValue* fh_openssl_csr_get_public_key(TypedValue* _rv, TypedValue* csr) asm("_ZN4HPHP28f_openssl_csr_get_public_keyERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_openssl_csr_get_subject(HPHP::Variant const&, bool)
_ZN4HPHP25f_openssl_csr_get_subjectERKNS_7VariantEb

(return value) => rax
_rv => rdi
csr => rsi
use_shortnames => rdx
*/

TypedValue* fh_openssl_csr_get_subject(TypedValue* _rv, TypedValue* csr, bool use_shortnames) asm("_ZN4HPHP25f_openssl_csr_get_subjectERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_openssl_csr_new(HPHP::Array const&, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP17f_openssl_csr_newERKNS_5ArrayERKNS_14VRefParamValueERKNS_7VariantES8_

(return value) => rax
_rv => rdi
dn => rsi
privkey => rdx
configargs => rcx
extraattribs => r8
*/

TypedValue* fh_openssl_csr_new(TypedValue* _rv, Value* dn, TypedValue* privkey, TypedValue* configargs, TypedValue* extraattribs) asm("_ZN4HPHP17f_openssl_csr_newERKNS_5ArrayERKNS_14VRefParamValueERKNS_7VariantES8_");

/*
HPHP::Variant HPHP::f_openssl_csr_sign(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, int, HPHP::Variant const&, int)
_ZN4HPHP18f_openssl_csr_signERKNS_7VariantES2_S2_iS2_i

(return value) => rax
_rv => rdi
csr => rsi
cacert => rdx
priv_key => rcx
days => r8
configargs => r9
serial => st0
*/

TypedValue* fh_openssl_csr_sign(TypedValue* _rv, TypedValue* csr, TypedValue* cacert, TypedValue* priv_key, int days, TypedValue* configargs, int serial) asm("_ZN4HPHP18f_openssl_csr_signERKNS_7VariantES2_S2_iS2_i");

/*
HPHP::Variant HPHP::f_openssl_error_string()
_ZN4HPHP22f_openssl_error_stringEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_openssl_error_string(TypedValue* _rv) asm("_ZN4HPHP22f_openssl_error_stringEv");

/*
bool HPHP::f_openssl_open(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP14f_openssl_openERKNS_6StringERKNS_14VRefParamValueES2_RKNS_7VariantE

(return value) => rax
sealed_data => rdi
open_data => rsi
env_key => rdx
priv_key_id => rcx
*/

bool fh_openssl_open(Value* sealed_data, TypedValue* open_data, Value* env_key, TypedValue* priv_key_id) asm("_ZN4HPHP14f_openssl_openERKNS_6StringERKNS_14VRefParamValueES2_RKNS_7VariantE");

/*
bool HPHP::f_openssl_pkcs12_export_to_file(HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP31f_openssl_pkcs12_export_to_fileERKNS_7VariantERKNS_6StringES2_S5_S2_

(return value) => rax
x509 => rdi
filename => rsi
priv_key => rdx
pass => rcx
args => r8
*/

bool fh_openssl_pkcs12_export_to_file(TypedValue* x509, Value* filename, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP31f_openssl_pkcs12_export_to_fileERKNS_7VariantERKNS_6StringES2_S5_S2_");

/*
bool HPHP::f_openssl_pkcs12_export(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP23f_openssl_pkcs12_exportERKNS_7VariantERKNS_14VRefParamValueES2_RKNS_6StringES2_

(return value) => rax
x509 => rdi
out => rsi
priv_key => rdx
pass => rcx
args => r8
*/

bool fh_openssl_pkcs12_export(TypedValue* x509, TypedValue* out, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP23f_openssl_pkcs12_exportERKNS_7VariantERKNS_14VRefParamValueES2_RKNS_6StringES2_");

/*
bool HPHP::f_openssl_pkcs12_read(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::String const&)
_ZN4HPHP21f_openssl_pkcs12_readERKNS_6StringERKNS_14VRefParamValueES2_

(return value) => rax
pkcs12 => rdi
certs => rsi
pass => rdx
*/

bool fh_openssl_pkcs12_read(Value* pkcs12, TypedValue* certs, Value* pass) asm("_ZN4HPHP21f_openssl_pkcs12_readERKNS_6StringERKNS_14VRefParamValueES2_");

/*
bool HPHP::f_openssl_pkcs7_decrypt(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP23f_openssl_pkcs7_decryptERKNS_6StringES2_RKNS_7VariantES5_

(return value) => rax
infilename => rdi
outfilename => rsi
recipcert => rdx
recipkey => rcx
*/

bool fh_openssl_pkcs7_decrypt(Value* infilename, Value* outfilename, TypedValue* recipcert, TypedValue* recipkey) asm("_ZN4HPHP23f_openssl_pkcs7_decryptERKNS_6StringES2_RKNS_7VariantES5_");

/*
bool HPHP::f_openssl_pkcs7_encrypt(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::Array const&, int, int)
_ZN4HPHP23f_openssl_pkcs7_encryptERKNS_6StringES2_RKNS_7VariantERKNS_5ArrayEii

(return value) => rax
infilename => rdi
outfilename => rsi
recipcerts => rdx
headers => rcx
flags => r8
cipherid => r9
*/

bool fh_openssl_pkcs7_encrypt(Value* infilename, Value* outfilename, TypedValue* recipcerts, Value* headers, int flags, int cipherid) asm("_ZN4HPHP23f_openssl_pkcs7_encryptERKNS_6StringES2_RKNS_7VariantERKNS_5ArrayEii");

/*
bool HPHP::f_openssl_pkcs7_sign(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, int, HPHP::String const&)
_ZN4HPHP20f_openssl_pkcs7_signERKNS_6StringES2_RKNS_7VariantES5_S5_iS2_

(return value) => rax
infilename => rdi
outfilename => rsi
signcert => rdx
privkey => rcx
headers => r8
flags => r9
extracerts => st0
*/

bool fh_openssl_pkcs7_sign(Value* infilename, Value* outfilename, TypedValue* signcert, TypedValue* privkey, TypedValue* headers, int flags, Value* extracerts) asm("_ZN4HPHP20f_openssl_pkcs7_signERKNS_6StringES2_RKNS_7VariantES5_S5_iS2_");

/*
HPHP::Variant HPHP::f_openssl_pkcs7_verify(HPHP::String const&, int, HPHP::String const&, HPHP::Array const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_openssl_pkcs7_verifyERKNS_6StringEiS2_RKNS_5ArrayES2_S2_

(return value) => rax
_rv => rdi
filename => rsi
flags => rdx
outfilename => rcx
cainfo => r8
extracerts => r9
content => st0
*/

TypedValue* fh_openssl_pkcs7_verify(TypedValue* _rv, Value* filename, int flags, Value* outfilename, Value* cainfo, Value* extracerts, Value* content) asm("_ZN4HPHP22f_openssl_pkcs7_verifyERKNS_6StringEiS2_RKNS_5ArrayES2_S2_");

/*
bool HPHP::f_openssl_pkey_export_to_file(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP29f_openssl_pkey_export_to_fileERKNS_7VariantERKNS_6StringES5_S2_

(return value) => rax
key => rdi
outfilename => rsi
passphrase => rdx
configargs => rcx
*/

bool fh_openssl_pkey_export_to_file(TypedValue* key, Value* outfilename, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP29f_openssl_pkey_export_to_fileERKNS_7VariantERKNS_6StringES5_S2_");

/*
bool HPHP::f_openssl_pkey_export(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21f_openssl_pkey_exportERKNS_7VariantERKNS_14VRefParamValueERKNS_6StringES2_

(return value) => rax
key => rdi
out => rsi
passphrase => rdx
configargs => rcx
*/

bool fh_openssl_pkey_export(TypedValue* key, TypedValue* out, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP21f_openssl_pkey_exportERKNS_7VariantERKNS_14VRefParamValueERKNS_6StringES2_");

/*
void HPHP::f_openssl_pkey_free(HPHP::Object const&)
_ZN4HPHP19f_openssl_pkey_freeERKNS_6ObjectE

key => rdi
*/

void fh_openssl_pkey_free(Value* key) asm("_ZN4HPHP19f_openssl_pkey_freeERKNS_6ObjectE");

/*
void HPHP::f_openssl_free_key(HPHP::Object const&)
_ZN4HPHP18f_openssl_free_keyERKNS_6ObjectE

key => rdi
*/

void fh_openssl_free_key(Value* key) asm("_ZN4HPHP18f_openssl_free_keyERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_openssl_pkey_get_details(HPHP::Object const&)
_ZN4HPHP26f_openssl_pkey_get_detailsERKNS_6ObjectE

(return value) => rax
_rv => rdi
key => rsi
*/

Value* fh_openssl_pkey_get_details(Value* _rv, Value* key) asm("_ZN4HPHP26f_openssl_pkey_get_detailsERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_openssl_pkey_get_private(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP26f_openssl_pkey_get_privateERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
key => rsi
passphrase => rdx
*/

TypedValue* fh_openssl_pkey_get_private(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP26f_openssl_pkey_get_privateERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_openssl_get_privatekey(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP24f_openssl_get_privatekeyERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
key => rsi
passphrase => rdx
*/

TypedValue* fh_openssl_get_privatekey(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP24f_openssl_get_privatekeyERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_openssl_pkey_get_public(HPHP::Variant const&)
_ZN4HPHP25f_openssl_pkey_get_publicERKNS_7VariantE

(return value) => rax
_rv => rdi
certificate => rsi
*/

TypedValue* fh_openssl_pkey_get_public(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP25f_openssl_pkey_get_publicERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_openssl_get_publickey(HPHP::Variant const&)
_ZN4HPHP23f_openssl_get_publickeyERKNS_7VariantE

(return value) => rax
_rv => rdi
certificate => rsi
*/

TypedValue* fh_openssl_get_publickey(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP23f_openssl_get_publickeyERKNS_7VariantE");

/*
HPHP::Object HPHP::f_openssl_pkey_new(HPHP::Variant const&)
_ZN4HPHP18f_openssl_pkey_newERKNS_7VariantE

(return value) => rax
_rv => rdi
configargs => rsi
*/

Value* fh_openssl_pkey_new(Value* _rv, TypedValue* configargs) asm("_ZN4HPHP18f_openssl_pkey_newERKNS_7VariantE");

/*
bool HPHP::f_openssl_private_decrypt(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP25f_openssl_private_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi

(return value) => rax
data => rdi
decrypted => rsi
key => rdx
padding => rcx
*/

bool fh_openssl_private_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

/*
bool HPHP::f_openssl_private_encrypt(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP25f_openssl_private_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi

(return value) => rax
data => rdi
crypted => rsi
key => rdx
padding => rcx
*/

bool fh_openssl_private_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

/*
bool HPHP::f_openssl_public_decrypt(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP24f_openssl_public_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi

(return value) => rax
data => rdi
decrypted => rsi
key => rdx
padding => rcx
*/

bool fh_openssl_public_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

/*
bool HPHP::f_openssl_public_encrypt(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP24f_openssl_public_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi

(return value) => rax
data => rdi
crypted => rsi
key => rdx
padding => rcx
*/

bool fh_openssl_public_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_openssl_seal(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP14f_openssl_sealERKNS_6StringERKNS_14VRefParamValueES5_RKNS_5ArrayE

(return value) => rax
_rv => rdi
data => rsi
sealed_data => rdx
env_keys => rcx
pub_key_ids => r8
*/

TypedValue* fh_openssl_seal(TypedValue* _rv, Value* data, TypedValue* sealed_data, TypedValue* env_keys, Value* pub_key_ids) asm("_ZN4HPHP14f_openssl_sealERKNS_6StringERKNS_14VRefParamValueES5_RKNS_5ArrayE");

/*
bool HPHP::f_openssl_sign(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP14f_openssl_signERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi

(return value) => rax
data => rdi
signature => rsi
priv_key_id => rdx
signature_alg => rcx
*/

bool fh_openssl_sign(Value* data, TypedValue* signature, TypedValue* priv_key_id, int signature_alg) asm("_ZN4HPHP14f_openssl_signERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_openssl_verify(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP16f_openssl_verifyERKNS_6StringES2_RKNS_7VariantEi

(return value) => rax
_rv => rdi
data => rsi
signature => rdx
pub_key_id => rcx
signature_alg => r8
*/

TypedValue* fh_openssl_verify(TypedValue* _rv, Value* data, Value* signature, TypedValue* pub_key_id, int signature_alg) asm("_ZN4HPHP16f_openssl_verifyERKNS_6StringES2_RKNS_7VariantEi");

/*
bool HPHP::f_openssl_x509_check_private_key(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP32f_openssl_x509_check_private_keyERKNS_7VariantES2_

(return value) => rax
cert => rdi
key => rsi
*/

bool fh_openssl_x509_check_private_key(TypedValue* cert, TypedValue* key) asm("_ZN4HPHP32f_openssl_x509_check_private_keyERKNS_7VariantES2_");

/*
long long HPHP::f_openssl_x509_checkpurpose(HPHP::Variant const&, int, HPHP::Array const&, HPHP::String const&)
_ZN4HPHP27f_openssl_x509_checkpurposeERKNS_7VariantEiRKNS_5ArrayERKNS_6StringE

(return value) => rax
x509cert => rdi
purpose => rsi
cainfo => rdx
untrustedfile => rcx
*/

long long fh_openssl_x509_checkpurpose(TypedValue* x509cert, int purpose, Value* cainfo, Value* untrustedfile) asm("_ZN4HPHP27f_openssl_x509_checkpurposeERKNS_7VariantEiRKNS_5ArrayERKNS_6StringE");

/*
bool HPHP::f_openssl_x509_export_to_file(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP29f_openssl_x509_export_to_fileERKNS_7VariantERKNS_6StringEb

(return value) => rax
x509 => rdi
outfilename => rsi
notext => rdx
*/

bool fh_openssl_x509_export_to_file(TypedValue* x509, Value* outfilename, bool notext) asm("_ZN4HPHP29f_openssl_x509_export_to_fileERKNS_7VariantERKNS_6StringEb");

/*
bool HPHP::f_openssl_x509_export(HPHP::Variant const&, HPHP::VRefParamValue const&, bool)
_ZN4HPHP21f_openssl_x509_exportERKNS_7VariantERKNS_14VRefParamValueEb

(return value) => rax
x509 => rdi
output => rsi
notext => rdx
*/

bool fh_openssl_x509_export(TypedValue* x509, TypedValue* output, bool notext) asm("_ZN4HPHP21f_openssl_x509_exportERKNS_7VariantERKNS_14VRefParamValueEb");

/*
void HPHP::f_openssl_x509_free(HPHP::Object const&)
_ZN4HPHP19f_openssl_x509_freeERKNS_6ObjectE

x509cert => rdi
*/

void fh_openssl_x509_free(Value* x509cert) asm("_ZN4HPHP19f_openssl_x509_freeERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_openssl_x509_parse(HPHP::Variant const&, bool)
_ZN4HPHP20f_openssl_x509_parseERKNS_7VariantEb

(return value) => rax
_rv => rdi
x509cert => rsi
shortnames => rdx
*/

TypedValue* fh_openssl_x509_parse(TypedValue* _rv, TypedValue* x509cert, bool shortnames) asm("_ZN4HPHP20f_openssl_x509_parseERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_openssl_x509_read(HPHP::Variant const&)
_ZN4HPHP19f_openssl_x509_readERKNS_7VariantE

(return value) => rax
_rv => rdi
x509certdata => rsi
*/

TypedValue* fh_openssl_x509_read(TypedValue* _rv, TypedValue* x509certdata) asm("_ZN4HPHP19f_openssl_x509_readERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_openssl_random_pseudo_bytes(int, HPHP::VRefParamValue const&)
_ZN4HPHP29f_openssl_random_pseudo_bytesEiRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
length => rsi
crypto_strong => rdx
*/

TypedValue* fh_openssl_random_pseudo_bytes(TypedValue* _rv, int length, TypedValue* crypto_strong) asm("_ZN4HPHP29f_openssl_random_pseudo_bytesEiRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_openssl_cipher_iv_length(HPHP::String const&)
_ZN4HPHP26f_openssl_cipher_iv_lengthERKNS_6StringE

(return value) => rax
_rv => rdi
method => rsi
*/

TypedValue* fh_openssl_cipher_iv_length(TypedValue* _rv, Value* method) asm("_ZN4HPHP26f_openssl_cipher_iv_lengthERKNS_6StringE");

/*
HPHP::Variant HPHP::f_openssl_encrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP17f_openssl_encryptERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
data => rsi
method => rdx
password => rcx
options => r8
iv => r9
*/

TypedValue* fh_openssl_encrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_encryptERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_openssl_decrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP17f_openssl_decryptERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
data => rsi
method => rdx
password => rcx
options => r8
iv => r9
*/

TypedValue* fh_openssl_decrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_decryptERKNS_6StringES2_S2_iS2_");

/*
HPHP::Variant HPHP::f_openssl_digest(HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP16f_openssl_digestERKNS_6StringES2_b

(return value) => rax
_rv => rdi
data => rsi
method => rdx
raw_output => rcx
*/

TypedValue* fh_openssl_digest(TypedValue* _rv, Value* data, Value* method, bool raw_output) asm("_ZN4HPHP16f_openssl_digestERKNS_6StringES2_b");

/*
HPHP::Array HPHP::f_openssl_get_cipher_methods(bool)
_ZN4HPHP28f_openssl_get_cipher_methodsEb

(return value) => rax
_rv => rdi
aliases => rsi
*/

Value* fh_openssl_get_cipher_methods(Value* _rv, bool aliases) asm("_ZN4HPHP28f_openssl_get_cipher_methodsEb");

/*
HPHP::Array HPHP::f_openssl_get_md_methods(bool)
_ZN4HPHP24f_openssl_get_md_methodsEb

(return value) => rax
_rv => rdi
aliases => rsi
*/

Value* fh_openssl_get_md_methods(Value* _rv, bool aliases) asm("_ZN4HPHP24f_openssl_get_md_methodsEb");


} // !HPHP

