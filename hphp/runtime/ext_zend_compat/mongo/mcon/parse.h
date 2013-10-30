/**
 *  Copyright 2009-2013 10gen, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef __MCON_PARSE_H__
#define __MCON_PARSE_H__

#include "types.h"

#define MONGO_SERVER_COPY_NONE        0x00
#define MONGO_SERVER_COPY_CREDENTIALS 0x01

/* Parsing server connection strings and its cleanup routines */
mongo_servers* mongo_parse_init(void);
int mongo_parse_server_spec(mongo_con_manager *manager, mongo_servers *servers, char *spec, char **error_message);
int mongo_store_option(mongo_con_manager *manager, mongo_servers *servers, char *option_name, char *option_value, char **error_message);
void mongo_servers_dump(mongo_con_manager *manager, mongo_servers *servers);
void mongo_servers_copy(mongo_servers *to, mongo_servers *from, int flags);
void mongo_server_def_dtor(mongo_server_def *server_def);
void mongo_servers_dtor(mongo_servers *servers);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
