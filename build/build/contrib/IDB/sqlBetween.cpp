#include "betweenIDB_SQL_TYPE.h"

#ifndef NO_SUCH_SQL

#include <vector>
#include <list>
#include "IDB/where.h"
#include <cppconn/prepared_statement.h>
#include "sqlBetweenIDB_SQL_TYPE.h"

#ifdef DEBUG_WITH_DMALLOC
#include "dmalloc.h"
#endif


namespace IDB { 
	sqlBetweenIDB_SQL_TYPE::sqlBetweenIDB_SQL_TYPE(std::string init_col, IDB_CXX_TYPE min_value, IDB_CXX_TYPE max_value) {
		col = init_col;
		min = min_value;
		max = max_value;
	}

	std::string sqlBetweenIDB_SQL_TYPE::toString() {
		#ifndef BETWEEN_SQL
			return col + " between ? and ?";
		#else
			return BETWEEN_SQL;
		#endif
	}

	#ifndef NO_BIND_NEEDED
		unsigned int sqlBetweenIDB_SQL_TYPE::bind(sql::PreparedStatement *pstmt, unsigned int bind_offset) {
			pstmt->setIDB_SQL_TYPE(bind_offset++, min);
			pstmt->setIDB_SQL_TYPE(bind_offset++, max);
	
			return bind_offset;
		}
	#endif
}

#endif
