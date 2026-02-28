/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* Error messages for MySQL clients */
/* (Error messages for the daemon are in share/language/errmsg.sys) */

#include "errmsg.h"
#include "my_sys.h"
#include "template_utils.h"

const char *client_errors[] = {
    "Unknown MySQL error",
    "Can't create UNIX socket (%d)",
    "Can't connect to local MySQL server through socket '%-.100s' (%d)",
    "Can't connect to MySQL server on '%-.100s' (%d)",
    "Can't create TCP/IP socket (%d)",
    "Unknown MySQL server host '%-.100s' (%d)",
    "MySQL server has gone away",
    "Protocol mismatch; server version = %d, client version = %d",
    "MySQL client ran out of memory",
    "Wrong host info",
    "Localhost via UNIX socket",
    "%-.100s via TCP/IP",
    "Error in server handshake",
    "Lost connection to MySQL server during query",
    "Commands out of sync; you can't run this command now",
    "Named pipe: %-.32s",
    "Can't wait for named pipe to host: %-.64s  pipe: %-.32s (%lu)",
    "Can't open named pipe to host: %-.64s  pipe: %-.32s (%lu)",
    "Can't set state of named pipe to host: %-.64s  pipe: %-.32s (%lu)",
    "Can't initialize character set %-.32s (path: %-.100s)",
    "Got packet bigger than 'max_allowed_packet' bytes",
    "Embedded server",
    "Error on SHOW SLAVE STATUS:",
    "Error on SHOW SLAVE HOSTS:",
    "Error connecting to slave:",
    "Error connecting to master:",
    "SSL connection error: %-.100s",
    "Malformed packet",
    "This client library is licensed only for use with MySQL servers having "
    "'%s' license",
    "Invalid use of null pointer",
    "Statement not prepared",
    "No data supplied for parameters in prepared statement",
    "Data truncated",
    "No parameters exist in the statement",
    "Invalid parameter number",
    "Can't send long data for non-string/non-binary data types (parameter: %d)",
    "Using unsupported buffer type: %d  (parameter: %d)",
    "Shared memory: %-.100s",
    "Can't open shared memory; client could not create request event (%lu)",
    "Can't open shared memory; no answer event received from server (%lu)",
    "Can't open shared memory; server could not allocate file mapping (%lu)",
    "Can't open shared memory; server could not get pointer to file mapping "
    "(%lu)",
    "Can't open shared memory; client could not allocate file mapping (%lu)",
    "Can't open shared memory; client could not get pointer to file mapping "
    "(%lu)",
    "Can't open shared memory; client could not create %s event (%lu)",
    "Can't open shared memory; no answer from server (%lu)",
    "Can't open shared memory; cannot send request event to server (%lu)",
    "Wrong or unknown protocol",
    "Invalid connection handle",
    "Connection using old (pre-4.1.1) authentication protocol refused (client "
    "option 'secure_auth' enabled)",
    "Row retrieval was canceled by mysql_stmt_close() call",
    "Attempt to read column without prior row fetch",
    "Prepared statement contains no metadata",
    "Attempt to read a row while there is no result set associated with the "
    "statement",
    "This feature is not implemented yet",
    "Lost connection to MySQL server at '%s', system error: %d",
    "Statement closed indirectly because of a preceding %s() call",
    "The number of columns in the result set differs from the number of bound "
    "buffers. You must reset the statement, rebind the result set columns, and "
    "execute the statement again",
    "This handle is already connected. Use a separate handle for each "
    "connection.",
    "Authentication plugin '%s' cannot be loaded: %s",
    "There is an attribute with the same name already",
    "Authentication plugin '%s' reported error: %s",
    "Insecure API function call: '%s' Use instead: '%s'",
    "File name is too long",
    "Set FIPS mode ON/STRICT failed",
    "Compression protocol not supported with asynchronous protocol",
    "Connection failed due to wrongly configured compression algorithm",
    "SSO user not found, Please perform SSO authentication using kerberos.",
    "Placeholder 2068",
    "Placeholder 2069",
    "Placeholder 2070",
    "Placeholder 2071",
    "Placeholder 2072",
    "Placeholder 2073",
    "Placeholder 2074",
    "Placeholder 2075",
    "Placeholder 2076",
    "Placeholder 2077",
    "Placeholder 2078",
    "Placeholder 2079",
    "Placeholder 2080",
    "Placeholder 2081",
    "Placeholder 2082",
    "Placeholder 2083",
    "Placeholder 2084",
    "Placeholder 2085",
    "Placeholder 2086",
    "Placeholder 2087",
    "Placeholder 2088",
    "Placeholder 2089",
    "Placeholder 2090",
    "Placeholder 2091",
    "Placeholder 2092",
    "Placeholder 2093",
    "Placeholder 2094",
    "Placeholder 2095",
    "Placeholder 2096",
    "Placeholder 2097",
    "Placeholder 2098",
    "Placeholder 2099",
    "Placeholder 2100",
    "Placeholder 2101",
    "Placeholder 2102",
    "Placeholder 2103",
    "Placeholder 2104",
    "Placeholder 2105",
    "Placeholder 2106",
    "Placeholder 2107",
    "Placeholder 2108",
    "Placeholder 2109",
    "Placeholder 2110",
    "Placeholder 2111",
    "Placeholder 2112",
    "Placeholder 2113",
    "Placeholder 2114",
    "Placeholder 2115",
    "Placeholder 2116",
    "Placeholder 2117",
    "Placeholder 2118",
    "Placeholder 2119",
    "Placeholder 2120",
    "Placeholder 2121",
    "Placeholder 2122",
    "Placeholder 2123",
    "Placeholder 2124",
    "Placeholder 2125",
    "Placeholder 2126",
    "Placeholder 2127",
    "Placeholder 2128",
    "Placeholder 2129",
    "Placeholder 2130",
    "Placeholder 2131",
    "Placeholder 2132",
    "Placeholder 2133",
    "Placeholder 2134",
    "Placeholder 2135",
    "Placeholder 2136",
    "Placeholder 2137",
    "Placeholder 2138",
    "Placeholder 2139",
    "Placeholder 2140",
    "Placeholder 2141",
    "Placeholder 2142",
    "Placeholder 2143",
    "Placeholder 2144",
    "Placeholder 2145",
    "Placeholder 2146",
    "Placeholder 2147",
    "Placeholder 2148",
    "Placeholder 2149",
    "Placeholder 2150",
    "Placeholder 2151",
    "Placeholder 2152",
    "Placeholder 2153",
    "Placeholder 2154",
    "Placeholder 2155",
    "Placeholder 2156",
    "Placeholder 2157",
    "Placeholder 2158",
    "Placeholder 2159",
    "Placeholder 2160",
    "Placeholder 2161",
    "Placeholder 2162",
    "Placeholder 2163",
    "Placeholder 2164",
    "Placeholder 2165",
    "Placeholder 2166",
    "Placeholder 2167",
    "Placeholder 2168",
    "Placeholder 2169",
    "Placeholder 2170",
    "Placeholder 2171",
    "Placeholder 2172",
    "Placeholder 2173",
    "Placeholder 2174",
    "Placeholder 2175",
    "Placeholder 2176",
    "Placeholder 2177",
    "Placeholder 2178",
    "Placeholder 2179",
    "Placeholder 2180",
    "Placeholder 2181",
    "Placeholder 2182",
    "Placeholder 2183",
    "Placeholder 2184",
    "Placeholder 2185",
    "Placeholder 2186",
    "Placeholder 2187",
    "Placeholder 2188",
    "Placeholder 2189",
    "Placeholder 2190",
    "Placeholder 2191",
    "Placeholder 2192",
    "Placeholder 2193",
    "Placeholder 2194",
    "Placeholder 2195",
    "Placeholder 2196",
    "Placeholder 2197",
    "Placeholder 2198",
    "Placeholder 2199",
    "Read timeout is reached",
    "Write timeout is reached",
    "Server rejected the ssl handshake for specified sni servername, sslerror: "
    "%-.100s",
    ""};

static const char *get_client_errmsg(int nr) {
  return client_errors[nr - CR_ERROR_FIRST];
}

/*
  Register client error messages for use with my_error().

  SYNOPSIS
    init_client_errs()

  RETURN
    void
*/

void init_client_errs(void) {
  static_assert(
      array_elements(client_errors) == (CR_ERROR_LAST - CR_ERROR_FIRST + 2),
      "");
  (void)my_error_register(get_client_errmsg, CR_ERROR_FIRST, CR_ERROR_LAST);
}

/*
  Unregister client error messages.

  SYNOPSIS
    finish_client_errs()

  RETURN
    void
*/

void finish_client_errs(void) {
  (void)my_error_unregister(CR_ERROR_FIRST, CR_ERROR_LAST);
}
