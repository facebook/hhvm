/*
    All rights reserved.

    Redistribution and use in any form are permitted provided that the
    following restrictions are are met:
        1.  Source distributions must retain this entire copyright notice
            and comment.
        2.  Binary distributions must include the acknowledgement ``This
            product includes software developed by Rich Salz'' in the
            documentation or other materials provided with the
            distribution.  This must not be represented as an endorsement
            or promotion without specific prior written permission.
        3.  The origin of this software must not be misrepresented, either
            by explicit claim or by omission.  Credits must appear in the
            source and documentation.
        4.  Altered versions must be plainly marked as such in the source
            and documentation and must not be misrepresented as being the
            original software.
    THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*
 * Copyright (C) 1986-1991 Rich Salz <rsalz@osf.org>
 *
 */

#ifndef incl_HPHP_WILDMAT_H_
#define incl_HPHP_WILDMAT_H_ 1

__BEGIN_DECLS

int wildmat(const char *text, const char *p);
int wildmatcase(const char *text, const char *p);

__END_DECLS

#endif /* incl_HPHP_WILDMAT_H_ */
