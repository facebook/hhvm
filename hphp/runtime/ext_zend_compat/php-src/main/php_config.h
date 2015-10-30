// So hacky...
#ifndef incl_HPHP_EZC_PHP_CONFIG_H
#define incl_HPHP_EZC_PHP_CONFIG_H

#ifdef __APPLE__
# include "php_config-darwin.h"
#elif defined(_MSC_VER)
# include "php_config-msvc.h"
#else
# include "php_config-linux.h"
#endif

#endif
