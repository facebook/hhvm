// So hacky...
#ifndef incl_HPHP_EZC_PHP_CONFIG_H
#define incl_HPHP_EZC_PHP_CONFIG_H

#ifdef __APPLE__
# include "php_config.h-darwin"
#else
# include "php_config.h-linux"
#endif

#endif
