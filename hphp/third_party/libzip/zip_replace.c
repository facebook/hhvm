/*
  zip_replace.c -- replace file via callback function
  Copyright (C) 1999-2009 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "zipint.h"



ZIP_EXTERN(int)
zip_replace(struct zip *za, zip_uint64_t idx, struct zip_source *source)
{
    if (idx >= za->nentry || source == NULL) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return -1;
    }

    if (_zip_replace(za, idx, NULL, source) == -1)
	return -1;

    return 0;
}




/* NOTE: Signed due to -1 on error.  See zip_add.c for more details. */

zip_int64_t
_zip_replace(struct zip *za, zip_uint64_t idx, const char *name,
	     struct zip_source *source)
{
    if (ZIP_IS_RDONLY(za)) {
	_zip_error_set(&za->error, ZIP_ER_RDONLY, 0);
	return -1;
    }

    if (idx == ZIP_UINT64_MAX) {
	if (_zip_entry_new(za) == NULL)
	    return -1;

	idx = za->nentry - 1;
    }
    
    _zip_unchange_data(za->entry+idx);

    if (name && _zip_set_name(za, idx, name) != 0)
	return -1;
    
    za->entry[idx].state = ((za->cdir == NULL || idx >= za->cdir->nentry)
			    ? ZIP_ST_ADDED : ZIP_ST_REPLACED);
    za->entry[idx].source = source;

    return idx;
}
