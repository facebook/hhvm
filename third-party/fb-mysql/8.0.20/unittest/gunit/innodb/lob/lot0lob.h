/*****************************************************************************

Copyright (c) 2016, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/
#ifndef _lot0lob_h_
#define _lot0lob_h_

#include <string.h>

#include "db0err.h"
#include "fil0types.h"
#include "fut0fut.h"
#include "fut0lst.h"
#include "lob0int.h"
#include "trx0types.h"

namespace lob {

struct ref_t {
  lobid_t m_id;
  bool operator<(const ref_t &that) const { return (m_id < that.m_id); }
  std::ostream &print(std::ostream &out) const {
    out << "[ref_t: m_id=" << m_id << "]";
    return (out);
  }
};

inline std::ostream &operator<<(std::ostream &out, const ref_t &obj) {
  return (obj.print(out));
}

/** Insert a large object (LOB) into the system.
@param[in,out]  ref  the LOB reference.
@param[in]  blob  the large object.
@param[in]  len  the length of the large object.*/
dberr_t insert(trx_id_t trxid, ref_t ref, byte *blob, ulint len);

/** Insert a large object (LOB) into the system.
@param[in,out]  ref  the LOB reference.
@param[in]  blob  the large object.
@param[in]  len  the length of the large object.*/
dberr_t z_insert(trx_id_t trxid, ref_t ref, byte *blob, ulint len);

/** Fetch a large object (LOB) from the system.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset read the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be fetched.
@param[out] buf  the output buffer (owned by caller) of minimum len bytes.
@return the amount of data (in bytes) that was actually read. */
ulint read(trx_id_t trxid, ref_t ref, ulint offset, ulint len, byte *buf);

/** Replace a large object (LOB) with the given new data.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset replace the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be replaced.
@param[in] buf  the buffer (owned by caller) with new data (len bytes).*/
dberr_t replace(trx_id_t trxid, ref_t ref, ulint offset, ulint len, byte *buf);

/** Insert data into the middle of an LOB */
ulint insert_middle(trx_id_t trxid, ref_t ref, ulint offset, byte *&data,
                    ulint &len);

/** Delete a portion of the given large object (LOB)
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset remove the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be removed. */
dberr_t remove_middle(trx_id_t trxid, ref_t ref, ulint offset, ulint len);

/** Remove/Delete/Destory the given LOB.
@param[in]  trxid  The transaction identifier.
@param[in]  ref  The LOB reference.*/
void remove(trx_id_t trxid, ref_t ref);

void trx_purge(trx_id_t trxid, ref_t ref);

void print(std::ostream &out, ref_t ref);

inline void trx_rollback(trx_id_t trxid, ref_t ref) { trx_purge(trxid, ref); }

}  // namespace lob

#endif  // _lot0lob_h_
