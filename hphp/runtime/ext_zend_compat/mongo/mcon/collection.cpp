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
#include "collection.h"
#include <stdlib.h>

mcon_collection *mcon_init_collection(int data_size)
{
	mcon_collection *c;

	c = (mcon_collection*) malloc(sizeof(mcon_collection));
	c->count = 0;
	c->space = 16;
	c->data_size = data_size;
	c->data = (void**) malloc(c->space * c->data_size);

	return c;
}

void mcon_collection_add(mcon_collection *c, void *data)
{
	if (c->count == c->space) {
		c->space = c->space * 2;
		c->data = (void**) realloc(c->data, c->space * c->data_size);
	}
	c->data[c->count] = data;
	c->count++;
}

void mcon_collection_iterate(mongo_con_manager *manager, mcon_collection *c, mcon_collection_callback_t cb)
{
	int i;

	for (i = 0; i < c->count; i++) {
		cb(manager, c->data[i]);
	}
}

void mcon_collection_free(mcon_collection *c)
{
	free(c->data);
	free(c);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
