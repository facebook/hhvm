
#ifndef incl_EXT_DBASE_H_
#define incl_EXT_DBASE_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_dbase_add_record(int dbase_identifier, CVarRef record);
bool f_dbase_close(int dbase_identifier);
Variant f_dbase_create(CStrRef filename, CVarRef fields);
bool f_dbase_delete_record(int dbase_identifier, int record_number);
Variant f_dbase_get_header_info(int dbase_identifier);
Variant f_dbase_get_record_with_names(int dbase_identifier, int record_number);
Variant f_dbase_get_record(int dbase_identifier, int record_number);
Variant f_dbase_numfields(int dbase_identifier);
Variant f_dbase_numrecords(int dbase_identifier);
Variant f_dbase_open(CStrRef filename, int mode);
bool f_dbase_pack(int dbase_identifier);
bool f_dbase_replace_record(int dbase_identifier, CVarRef record, int record_number);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_DBASE_H_
