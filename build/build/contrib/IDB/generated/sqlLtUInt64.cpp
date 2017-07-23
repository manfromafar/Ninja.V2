#include "sqlLtUInt64.h"
#include "opLt.h"
#include <cppconn/prepared_statement.h>

#ifdef DEBUG_WITH_DMALLOC
#include "dmalloc.h"
#endif


namespace IDB { 
	std::string sqlLtUInt64::toString() {
		std::string			output;

		output = col + IDB_SQL_OP + "?";

		return output;
	}

	unsigned int sqlLtUInt64::bind(sql::PreparedStatement *pstmt, unsigned int bind_offset) {
		pstmt->setUInt64(bind_offset++, value);

		return bind_offset;
	}
}
