#ifndef TSRM_CONFIG_W32_H
#define TSRM_CONFIG_W32_H

#include "../main/php_config.h"
#include "../Zend/zend_config.w32.h"

#define HAVE_UTIME 1
#define HAVE_ALLOCA 1
#define HAVE_REALPATH 1

#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

#endif
