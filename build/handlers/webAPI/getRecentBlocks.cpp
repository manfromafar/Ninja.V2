#include "webAPI/getRecentBlocks.hpp"

#include "mining-info.hpp"

#include "Nonce.hpp"
#include "Block.hpp"

#include "cJSON.hpp"

#ifdef DEBUG_WITH_DMALLOC
#include "dmalloc.h"
#endif


static void add_account_info( cJSON *root, std::string prefix, uint64_t accountID ) {
	// don't use "update_check()" here as it massively slows down the whole server
	// there'll be a separate thread to update these accounts
	Account *account = Account::load_or_create(accountID);

	if (account) {
		cJSON_AddStringToObject( root, std::string( prefix + "Account").c_str(), account->account_RS_string().c_str() );
		cJSON_AddStringToObject( root, std::string( prefix + "AccountId").c_str(), std::to_string(accountID).c_str() );
		cJSON_AddNumberToObject( root, std::string( prefix + "AccountId32").c_str(), accountID & 0xFFFFFFFF );
		cJSON_AddStringToObject( root, std::string( prefix + "AccountName").c_str(), account->account_name().c_str() );

		delete account;
	} else {
		cJSON_AddStringToObject( root, std::string( prefix + "Account").c_str(), (char *)"???" );
		cJSON_AddStringToObject( root, std::string( prefix + "AccountId").c_str(), (char *)"0" );
		cJSON_AddNumberToObject( root, std::string( prefix + "AccountId32").c_str(), 0 );
		cJSON_AddStringToObject( root, std::string( prefix + "AccountName").c_str(), (char *)"???" );
	}
}


int Handlers::webAPI::getRecentBlocks::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	uint64_t latest_blockID = Block::latest_blockID();

	Block blocks;
	blocks.order_by( Block::COL_blockID, Block::ORDER_BY_DESC );
	blocks.after_blockID( latest_blockID - RECENT_BLOCK_HISTORY_DEPTH );
	blocks.before_blockID( latest_blockID );
	blocks.search();

	cJSON *historic_array = cJSON_CreateArray();

	while( Block *block = blocks.result() ) {
		cJSON *entry = cJSON_CreateObject();
		cJSON_AddNumberToObject( entry, (char *)"block", block->blockID() );

		add_account_info( entry, "generator", block->generator_account_id() );

		if ( block->best_nonce_account_id() ) {
			add_account_info( entry, "ourBest", block->best_nonce_account_id() );
			cJSON_AddNumberToObject( entry, (char *)"ourBestDeadline", block->our_best_deadline() );
		}

		cJSON_AddNumberToObject( entry, (char *)"blockReward", block->block_reward() );
		cJSON_AddNumberToObject( entry, (char *)"isOurBlock", block->is_our_block() );
		cJSON_AddNumberToObject( entry, (char *)"deadline", block->deadline() );
		cJSON_AddItemToArray(historic_array, entry);

		delete block;
	}

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject( root, (char *)"blocks", historic_array );

	std::unique_ptr<char> json( cJSON_PrintUnformatted(root) );

	resp->status_code = 200;
	resp->content = std::string(&*json);
	resp->add_header( "Access-Control-Allow-Origin", "*" );

	cJSON_Delete(root);

	return MHD_YES;
}
