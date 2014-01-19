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
#ifndef __MCON_IO_H__
#define __MCON_IO_H__
#include "types.h"

int mongo_io_wait_with_timeout(int sock, int to, char **error_message);
int mongo_io_send(mongo_connection *con, mongo_server_options *options, void *data, int size, char **error_message);
int mongo_io_recv_header(mongo_connection *con, mongo_server_options *options, int timeout, void *data, int size, char **error_message);
int mongo_io_recv_data(mongo_connection *con, mongo_server_options *options, int timeout, void *data, int size, char **error_message);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
