#if !defined(__EXT_PHPMCC_ACCESSORS_H__)
#define __EXT_PHPMCC_ACCESSORS_H__

#include "types.h"

namespace HPHP {

extern Array get_serverpool_servers(mcc_handle_t mcc,
                                    const nstring_t* serverpool,
                                    bool is_mirror);
extern Array get_accesspoints(mcc_handle_t mcc, const nstring_t* server);
extern Variant phpmcc_read_property(MccResourcePtr &phpmcc, CVarRef member);
extern void phpmcc_write_property(MccResourcePtr &phpmcc, CVarRef member,
                                  CVarRef value);

///////////////////////////////////////////////////////////////////////////////
}

#endif /* #if !defined(__EXT_PHPMCC_ACCESSORS_H__) */
