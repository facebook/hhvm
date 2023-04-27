/*
 *  Fedora supports multi arch: having 32 and 64 versions of MySQL
 *  installed at the same time. my_config.h will differ due arch
 *  dependent defs creating a file conflict. We move arch specific
 *  headers to arch specific file names and include the correct arch
 *  specific file by installing this generic file.
 *
 */

#if defined(__i386__)
#include "my_config_i386.h"
#elif defined(__ia64__)
#include "my_config_ia64.h"
#elif defined(__powerpc__)
#include "my_config_ppc.h"
#elif defined(__powerpc64__)
#include "my_config_ppc64.h"
#elif defined(__s390x__)
#include "my_config_s390x.h"
#elif defined(__s390__)
#include "my_config_s390.h"
#elif defined(__sparc__) && defined(__arch64__)
#include "my_config_sparc64.h"
#elif defined(__sparc__)
#include "my_config_sparc.h"
#elif defined(__x86_64__)
#include "my_config_x86_64.h"
#else
#error "This MySQL devel package does not work your architecture?"
#endif
