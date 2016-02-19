#ifndef incl_HPHP_PDO_PGSQL_RESOURCE_H_
#define incl_HPHP_PDO_PGSQL_RESOURCE_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pdo_pgsql_connection.h"
#include "pq.h"
#include "stdarg.h"


namespace HPHP {
    class PDOPgSqlResource : public PDOResource {

    public:
        explicit PDOPgSqlResource(std::shared_ptr<PDOPgSqlConnection> conn)
        : PDOResource(std::dynamic_pointer_cast<PDOConnection>(conn))
        {}

        std::shared_ptr<PDOPgSqlConnection> conn() const {
            return std::dynamic_pointer_cast<PDOPgSqlConnection>(m_conn);
        }
    };
}

#endif
