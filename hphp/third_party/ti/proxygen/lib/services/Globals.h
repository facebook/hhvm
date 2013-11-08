// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include <ti/proxygen/lib/utils/Exception.h>
#include <ti/proxygen/lib/utils/Util.h>

#include <string>

namespace facebook { namespace proxygen {

extern std::string localHostname, localIp;

/**
 * Get the current date and time in string formats: %Y-%m-%d and
 * %H:%M:%S.
 */
extern void getDateTimeStr(char datebuf[32], char timebuf[32]);

}}
