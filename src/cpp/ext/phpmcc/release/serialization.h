#if !defined(__EXT_PHPMCC_SERIALIZATION_H__)
#define __EXT_PHPMCC_SERIALIZATION_H__

#include "types.h"

namespace HPHP {

extern bool phpmcc_fb_serialize_value(MccResourcePtr &phpmcc,
                                      CStrRef key,
                                      phpmcc_flags_t& flags,
                                      CVarRef unserialized_value,
                                      String& serialized_value);
extern bool phpmcc_php_serialize_value(MccResourcePtr &phpmcc,
                                       CStrRef key,
                                       phpmcc_flags_t& flags,
                                       CVarRef unserialized_value,
                                       String& serialized_value);
extern int phpmcc_unserialize_value(MccResourcePtr &phpmcc,
                                    const phpmcc_flags_t flags,
                                    const char *serialized_value,
                                    int serialized_len,
                                    Variant& unserialized_value);
extern int phpmcc_zlib_compress_value(MccResourcePtr &phpmcc,
                                      phpmcc_flags_t& flags,
                                      CStrRef uncompressed_value,
                                      String& compressed_value);
extern int phpmcc_zlib_uncompress_value(MccResourcePtr &phpmcc,
                                        uint32_t flags,
                                        const char *compressed_value,
                                        int compressed_len,
                                        String& uncompressed_value);

///////////////////////////////////////////////////////////////////////////////
}

#endif /* #if !defined(__EXT_PHPMCC_SERIALIZATION_H__) */
