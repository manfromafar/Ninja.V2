/*
		Create table Shares (
			blockID						bigint unsigned not null,
			accountID					bigint unsigned not null,
			share_fraction				double not null,
			deadline					bigint unsigned not null,
			deadline_string				varchar(255) not null,
			miner						varchar(255),
			primary key					(blockID, accountID),
			index						(blockID, share_fraction)
		);
*/

#include "Share.cxx"

SEARCHMOD( before_blockID, uint64_t );
SEARCHMOD( after_blockID, uint64_t );
SEARCHMOD( historic_average, bool );
SEARCHMOD( mean_weighted_deadline, bool );


SEARCHPREP {
	Block *blocks = NULL;
	bool delete_additional_on_exit = false;

	if ( SEARCHMOD_IS_SET(mean_weighted_deadline) ) {
		blocks = new Block;

		if ( SEARCHMOD_IS_SET(before_blockID) )
			blocks->before_blockID( SEARCHMOD_VALUE(before_blockID) );

		if ( SEARCHMOD_IS_SET(after_blockID) )
			blocks->after_blockID( SEARCHMOD_VALUE(after_blockID) );

		if (!additional) {
			additional = new std::vector<IO::Object *>;
			delete_additional_on_exit = true;
		}

		additional->push_back( blocks );
	}

	SEARCHPREP_SUPER;

	if ( SEARCHMOD_IS_SET(before_blockID) ) {
		IDB::Where *new_clause = new IDB::sqlLtUInt64( "Shares.blockID", SEARCHMOD_VALUE(before_blockID) );
		SEARCHPREP_ADD( new_clause );
	}

	if ( SEARCHMOD_IS_SET(after_blockID) ) {
		IDB::Where *new_clause = new IDB::sqlGtUInt64( "Shares.blockID", SEARCHMOD_VALUE(after_blockID) );
		SEARCHPREP_ADD( new_clause );
	}

	if ( SEARCHMOD_IS_SET(historic_average) ) {
		ps->options->order_by = "share_fraction DESC";
		ps->options->group_by = "accountID";

		ps->cols->clear();
		ps->cols->push_back("blockID");
		ps->cols->push_back("accountID");

		// this is gross:
		std::string sub_select = "select count(distinct blockID) from Shares where blockID > " + std::to_string( SEARCHMOD_VALUE(after_blockID) ) + " and blockID < " + std::to_string( SEARCHMOD_VALUE(before_blockID) );
		ps->cols->push_back( "sum(share_fraction)/(" + sub_select + ") as share_fraction" );

		ps->cols->push_back("null");
		ps->cols->push_back("null");
		ps->cols->push_back("null");
}

	if ( SEARCHMOD_IS_SET(mean_weighted_deadline) ) {
		ps->options->order_by = "weighted_deadline ASC";

		ps->cols->clear();
		ps->cols->push_back("Shares.blockID");
		ps->cols->push_back("Shares.accountID");
		ps->cols->push_back("null");
		// change deadline col
		ps->cols->push_back( "Shares.deadline*Blocks.base_target as weighted_deadline" );
		ps->cols->push_back("null");
		ps->cols->push_back("null");
	}

	if (blocks)
		delete blocks;

	if (additional && delete_additional_on_exit)
		delete additional;

	SEARCHPREP_END;
}


STATIC Share *Share::historic_shares( uint64_t to_blockID, uint64_t block_count ) {
	Share *shares = new Share();
	shares->after_blockID( to_blockID - block_count );
	// include this block
	shares->before_blockID( to_blockID + 1 );
	shares->historic_average(true);
	return shares;
}
