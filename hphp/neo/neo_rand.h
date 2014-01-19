/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#ifndef incl_HPHP_NEO_RAND_H_
#define incl_HPHP_NEO_RAND_H_ 1

__BEGIN_DECLS

void neo_seed_rand (long int seed);
int neo_rand (int max);
int neo_rand_string (char *s, int slen);
int neo_rand_word (char *s, int slen);

__END_DECLS

#endif /* incl_HPHP_NEO_RAND_H_ */
