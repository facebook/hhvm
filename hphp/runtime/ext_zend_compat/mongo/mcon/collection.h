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
#ifndef __MCON_COLLECTION_H__
#define __MCON_COLLECTION_H__

#include "types.h"

mcon_collection *mcon_init_collection(int data_size);
void mcon_collection_add(mcon_collection *c, void *data);
void mcon_collection_iterate(mongo_con_manager *manager, mcon_collection *c, mcon_collection_callback_t cb);
void mcon_collection_free(mcon_collection *c);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
