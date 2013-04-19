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

bool fh_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* additional_parameters) asm("_ZN4HPHP6f_mailERKNS_6StringES2_S2_S2_S2_");

long fh_ezmlm_hash(Value* addr) asm("_ZN4HPHP12f_ezmlm_hashERKNS_6StringE");

Value* fh_mailparse_msg_create(Value* _rv) asm("_ZN4HPHP22f_mailparse_msg_createEv");

bool fh_mailparse_msg_free(Value* mimemail) asm("_ZN4HPHP20f_mailparse_msg_freeERKNS_6ObjectE");

TypedValue* fh_mailparse_msg_parse_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP26f_mailparse_msg_parse_fileERKNS_6StringE");

bool fh_mailparse_msg_parse(Value* mimemail, Value* data) asm("_ZN4HPHP21f_mailparse_msg_parseERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_mailparse_msg_extract_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP33f_mailparse_msg_extract_part_fileERKNS_6ObjectERKNS_7VariantES5_");

TypedValue* fh_mailparse_msg_extract_whole_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP39f_mailparse_msg_extract_whole_part_fileERKNS_6ObjectERKNS_7VariantES5_");

TypedValue* fh_mailparse_msg_extract_part(TypedValue* _rv, Value* mimemail, TypedValue* msgbody, TypedValue* callbackfunc) asm("_ZN4HPHP28f_mailparse_msg_extract_partERKNS_6ObjectERKNS_7VariantES5_");

Value* fh_mailparse_msg_get_part_data(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_part_dataERKNS_6ObjectE");

TypedValue* fh_mailparse_msg_get_part(TypedValue* _rv, Value* mimemail, Value* mimesection) asm("_ZN4HPHP24f_mailparse_msg_get_partERKNS_6ObjectERKNS_6StringE");

Value* fh_mailparse_msg_get_structure(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_structureERKNS_6ObjectE");

Value* fh_mailparse_rfc822_parse_addresses(Value* _rv, Value* addresses) asm("_ZN4HPHP34f_mailparse_rfc822_parse_addressesERKNS_6StringE");

bool fh_mailparse_stream_encode(Value* sourcefp, Value* destfp, Value* encoding) asm("_ZN4HPHP25f_mailparse_stream_encodeERKNS_6ObjectES2_RKNS_6StringE");

TypedValue* fh_mailparse_uudecode_all(TypedValue* _rv, Value* fp) asm("_ZN4HPHP24f_mailparse_uudecode_allERKNS_6ObjectE");

TypedValue* fh_mailparse_determine_best_xfer_encoding(TypedValue* _rv, Value* fp) asm("_ZN4HPHP40f_mailparse_determine_best_xfer_encodingERKNS_6ObjectE");

} // namespace HPHP
