#ifndef ERRMSG_INCLUDED
#define ERRMSG_INCLUDED

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

/**
  @file include/errmsg.h

  Error messages for MySQL clients.
  These are constant and use the CR_ prefix.
  <mysqlclient_ername.h> will contain auto-generated mappings
  containing the symbolic name and the number from this file,
  and the english error messages in libmysql/errmsg.c.

  Dynamic error messages for the daemon are in share/language/errmsg.sys.
  The server equivalent to <errmsg.h> is <mysqld_error.h>.
  The server equivalent to <mysqlclient_ername.h> is <mysqld_ername.h>.

  Note that the auth subsystem also uses codes with a CR_ prefix.
*/

void init_client_errs(void);
void finish_client_errs(void);
extern const char *client_errors[]; /* Error messages */

#define CR_MIN_ERROR 2000 /* For easier client code */
#define CR_MAX_ERROR 2999
#define CLIENT_ERRMAP 2 /* Errormap used by my_error() */

/* Do not add error numbers before CR_ERROR_FIRST. */
/* If necessary to add lower numbers, change CR_ERROR_FIRST accordingly. */
#define CR_ERROR_FIRST 2000 /*Copy first error nr.*/
#define CR_UNKNOWN_ERROR 2000
#define CR_SOCKET_CREATE_ERROR 2001
#define CR_CONNECTION_ERROR 2002
#define CR_CONN_HOST_ERROR 2003
#define CR_IPSOCK_ERROR 2004
#define CR_UNKNOWN_HOST 2005
#define CR_SERVER_GONE_ERROR 2006
#define CR_VERSION_ERROR 2007
#define CR_OUT_OF_MEMORY 2008
#define CR_WRONG_HOST_INFO 2009
#define CR_LOCALHOST_CONNECTION 2010
#define CR_TCP_CONNECTION 2011
#define CR_SERVER_HANDSHAKE_ERR 2012
#define CR_SERVER_LOST 2013
#define CR_COMMANDS_OUT_OF_SYNC 2014
#define CR_NAMEDPIPE_CONNECTION 2015
#define CR_NAMEDPIPEWAIT_ERROR 2016
#define CR_NAMEDPIPEOPEN_ERROR 2017
#define CR_NAMEDPIPESETSTATE_ERROR 2018
#define CR_CANT_READ_CHARSET 2019
#define CR_NET_PACKET_TOO_LARGE 2020
#define CR_EMBEDDED_CONNECTION 2021
#define CR_PROBE_SLAVE_STATUS 2022
#define CR_PROBE_SLAVE_HOSTS 2023
#define CR_PROBE_SLAVE_CONNECT 2024
#define CR_PROBE_MASTER_CONNECT 2025
#define CR_SSL_CONNECTION_ERROR 2026
#define CR_MALFORMED_PACKET 2027
#define CR_WRONG_LICENSE 2028

/* new 4.1 error codes */
#define CR_NULL_POINTER 2029
#define CR_NO_PREPARE_STMT 2030
#define CR_PARAMS_NOT_BOUND 2031
#define CR_DATA_TRUNCATED 2032
#define CR_NO_PARAMETERS_EXISTS 2033
#define CR_INVALID_PARAMETER_NO 2034
#define CR_INVALID_BUFFER_USE 2035
#define CR_UNSUPPORTED_PARAM_TYPE 2036

#define CR_SHARED_MEMORY_CONNECTION 2037
#define CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR 2038
#define CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR 2039
#define CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR 2040
#define CR_SHARED_MEMORY_CONNECT_MAP_ERROR 2041
#define CR_SHARED_MEMORY_FILE_MAP_ERROR 2042
#define CR_SHARED_MEMORY_MAP_ERROR 2043
#define CR_SHARED_MEMORY_EVENT_ERROR 2044
#define CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR 2045
#define CR_SHARED_MEMORY_CONNECT_SET_ERROR 2046
#define CR_CONN_UNKNOW_PROTOCOL 2047
#define CR_INVALID_CONN_HANDLE 2048
#define CR_UNUSED_1 2049
#define CR_FETCH_CANCELED 2050
#define CR_NO_DATA 2051
#define CR_NO_STMT_METADATA 2052
#define CR_NO_RESULT_SET 2053
#define CR_NOT_IMPLEMENTED 2054
#define CR_SERVER_LOST_EXTENDED 2055
#define CR_STMT_CLOSED 2056
#define CR_NEW_STMT_METADATA 2057
#define CR_ALREADY_CONNECTED 2058
#define CR_AUTH_PLUGIN_CANNOT_LOAD 2059
#define CR_DUPLICATE_CONNECTION_ATTR 2060
#define CR_AUTH_PLUGIN_ERR 2061
#define CR_INSECURE_API_ERR 2062
#define CR_FILE_NAME_TOO_LONG 2063
#define CR_SSL_FIPS_MODE_ERR 2064
#define CR_COMPRESSION_NOT_SUPPORTED 2065
#define CR_COMPRESSION_WRONGLY_CONFIGURED 2066
#define CR_KERBEROS_USER_NOT_FOUND 2067
#define CR_PLACEHOLDER_2068 2068
#define CR_PLACEHOLDER_2069 2069
#define CR_PLACEHOLDER_2070 2070
#define CR_PLACEHOLDER_2071 2071
#define CR_PLACEHOLDER_2072 2072
#define CR_PLACEHOLDER_2073 2073
#define CR_PLACEHOLDER_2074 2074
#define CR_PLACEHOLDER_2075 2075
#define CR_PLACEHOLDER_2076 2076
#define CR_PLACEHOLDER_2077 2077
#define CR_PLACEHOLDER_2078 2078
#define CR_PLACEHOLDER_2079 2079
#define CR_PLACEHOLDER_2080 2080
#define CR_PLACEHOLDER_2081 2081
#define CR_PLACEHOLDER_2082 2082
#define CR_PLACEHOLDER_2083 2083
#define CR_PLACEHOLDER_2084 2084
#define CR_PLACEHOLDER_2085 2085
#define CR_PLACEHOLDER_2086 2086
#define CR_PLACEHOLDER_2087 2087
#define CR_PLACEHOLDER_2088 2088
#define CR_PLACEHOLDER_2089 2089
#define CR_PLACEHOLDER_2090 2090
#define CR_PLACEHOLDER_2091 2091
#define CR_PLACEHOLDER_2092 2092
#define CR_PLACEHOLDER_2093 2093
#define CR_PLACEHOLDER_2094 2094
#define CR_PLACEHOLDER_2095 2095
#define CR_PLACEHOLDER_2096 2096
#define CR_PLACEHOLDER_2097 2097
#define CR_PLACEHOLDER_2098 2098
#define CR_PLACEHOLDER_2099 2099
#define CR_PLACEHOLDER_2100 2100
#define CR_PLACEHOLDER_2101 2101
#define CR_PLACEHOLDER_2102 2102
#define CR_PLACEHOLDER_2103 2103
#define CR_PLACEHOLDER_2104 2104
#define CR_PLACEHOLDER_2105 2105
#define CR_PLACEHOLDER_2106 2106
#define CR_PLACEHOLDER_2107 2107
#define CR_PLACEHOLDER_2108 2108
#define CR_PLACEHOLDER_2109 2109
#define CR_PLACEHOLDER_2110 2110
#define CR_PLACEHOLDER_2111 2111
#define CR_PLACEHOLDER_2112 2112
#define CR_PLACEHOLDER_2113 2113
#define CR_PLACEHOLDER_2114 2114
#define CR_PLACEHOLDER_2115 2115
#define CR_PLACEHOLDER_2116 2116
#define CR_PLACEHOLDER_2117 2117
#define CR_PLACEHOLDER_2118 2118
#define CR_PLACEHOLDER_2119 2119
#define CR_PLACEHOLDER_2120 2120
#define CR_PLACEHOLDER_2121 2121
#define CR_PLACEHOLDER_2122 2122
#define CR_PLACEHOLDER_2123 2123
#define CR_PLACEHOLDER_2124 2124
#define CR_PLACEHOLDER_2125 2125
#define CR_PLACEHOLDER_2126 2126
#define CR_PLACEHOLDER_2127 2127
#define CR_PLACEHOLDER_2128 2128
#define CR_PLACEHOLDER_2129 2129
#define CR_PLACEHOLDER_2130 2130
#define CR_PLACEHOLDER_2131 2131
#define CR_PLACEHOLDER_2132 2132
#define CR_PLACEHOLDER_2133 2133
#define CR_PLACEHOLDER_2134 2134
#define CR_PLACEHOLDER_2135 2135
#define CR_PLACEHOLDER_2136 2136
#define CR_PLACEHOLDER_2137 2137
#define CR_PLACEHOLDER_2138 2138
#define CR_PLACEHOLDER_2139 2139
#define CR_PLACEHOLDER_2140 2140
#define CR_PLACEHOLDER_2141 2141
#define CR_PLACEHOLDER_2142 2142
#define CR_PLACEHOLDER_2143 2143
#define CR_PLACEHOLDER_2144 2144
#define CR_PLACEHOLDER_2145 2145
#define CR_PLACEHOLDER_2146 2146
#define CR_PLACEHOLDER_2147 2147
#define CR_PLACEHOLDER_2148 2148
#define CR_PLACEHOLDER_2149 2149
#define CR_PLACEHOLDER_2150 2150
#define CR_PLACEHOLDER_2151 2151
#define CR_PLACEHOLDER_2152 2152
#define CR_PLACEHOLDER_2153 2153
#define CR_PLACEHOLDER_2154 2154
#define CR_PLACEHOLDER_2155 2155
#define CR_PLACEHOLDER_2156 2156
#define CR_PLACEHOLDER_2157 2157
#define CR_PLACEHOLDER_2158 2158
#define CR_PLACEHOLDER_2159 2159
#define CR_PLACEHOLDER_2160 2160
#define CR_PLACEHOLDER_2161 2161
#define CR_PLACEHOLDER_2162 2162
#define CR_PLACEHOLDER_2163 2163
#define CR_PLACEHOLDER_2164 2164
#define CR_PLACEHOLDER_2165 2165
#define CR_PLACEHOLDER_2166 2166
#define CR_PLACEHOLDER_2167 2167
#define CR_PLACEHOLDER_2168 2168
#define CR_PLACEHOLDER_2169 2169
#define CR_PLACEHOLDER_2170 2170
#define CR_PLACEHOLDER_2171 2171
#define CR_PLACEHOLDER_2172 2172
#define CR_PLACEHOLDER_2173 2173
#define CR_PLACEHOLDER_2174 2174
#define CR_PLACEHOLDER_2175 2175
#define CR_PLACEHOLDER_2176 2176
#define CR_PLACEHOLDER_2177 2177
#define CR_PLACEHOLDER_2178 2178
#define CR_PLACEHOLDER_2179 2179
#define CR_PLACEHOLDER_2180 2180
#define CR_PLACEHOLDER_2181 2181
#define CR_PLACEHOLDER_2182 2182
#define CR_PLACEHOLDER_2183 2183
#define CR_PLACEHOLDER_2184 2184
#define CR_PLACEHOLDER_2185 2185
#define CR_PLACEHOLDER_2186 2186
#define CR_PLACEHOLDER_2187 2187
#define CR_PLACEHOLDER_2188 2188
#define CR_PLACEHOLDER_2189 2189
#define CR_PLACEHOLDER_2190 2190
#define CR_PLACEHOLDER_2191 2191
#define CR_PLACEHOLDER_2192 2192
#define CR_PLACEHOLDER_2193 2193
#define CR_PLACEHOLDER_2194 2194
#define CR_PLACEHOLDER_2195 2195
#define CR_PLACEHOLDER_2196 2196
#define CR_PLACEHOLDER_2197 2197
#define CR_PLACEHOLDER_2198 2198
#define CR_PLACEHOLDER_2199 2199
// Facebook client errors
#define CR_NET_READ_INTERRUPTED 2200
#define CR_NET_WRITE_INTERRUPTED 2201
#define CR_TLS_SERVER_NOT_FOUND 2202
#define CR_ERROR_LAST /*Copy last error nr:*/ 2202
/* Add error numbers before CR_ERROR_LAST and change it accordingly. */

#define CR_PLACEHOLDER_FIRST CR_PLACEHOLDER_2068
#define CR_PLACEHOLDER_LAST CR_PLACEHOLDER_2199

static inline bool isPlaceHolder(int client_errno) {
  return client_errno >= CR_PLACEHOLDER_FIRST &&
         client_errno <= CR_PLACEHOLDER_LAST;
}

/* Visual Studio requires '__inline' for C code */
static inline const char *ER_CLIENT(int client_errno) {
  if (client_errno >= CR_ERROR_FIRST && client_errno <= CR_ERROR_LAST &&
      !isPlaceHolder(client_errno)) {
    return client_errors[client_errno - CR_ERROR_FIRST];
  }
  return client_errors[CR_UNKNOWN_ERROR - CR_ERROR_FIRST];
}

#endif /* ERRMSG_INCLUDED */
