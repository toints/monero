//
// Created by mac on 2019/4/27.
//

#include <misc_log_ex.h>
#include "db_sqlite3.h"


	void SQLITE_UINT_BIG_THAN( sqlite3_context* ctx,int argc, sqlite3_value** argv )
	{
		uint64_t value1 = *(uint64_t*)(sqlite3_value_blob(argv[0]));
		uint64_t value2 = *(uint64_t*)(sqlite3_value_blob(argv[1]));
//		LOG_PRINT_L0("blob value1  " << value1);
//		LOG_PRINT_L0("blob value2  " << value2);

		sqlite3_result_int( ctx, value1 > value2 );
	}

	void SQLITE_UINT_BIG_EQUAL_THAN( sqlite3_context* ctx,int argc, sqlite3_value** argv )
	{
		uint64_t value1 = *(uint64_t*)(sqlite3_value_blob(argv[0]));
		uint64_t value2 = *(uint64_t*)(sqlite3_value_blob(argv[1]));
//		LOG_PRINT_L0("blob value1  " << value1);
//		LOG_PRINT_L0("blob value2  " << value2);

		sqlite3_result_int( ctx, value1 >= value2 );
	}

	void SQLITE_UINT_SMALL_THAN( sqlite3_context* ctx,int argc, sqlite3_value** argv )
	{
		uint64_t value1 = *(uint64_t*)(sqlite3_value_blob(argv[0]));
		uint64_t value2 = *(uint64_t*)(sqlite3_value_blob(argv[1]));
//		LOG_PRINT_L0("blob value1  " << value1);
//		LOG_PRINT_L0("blob value2  " << value2);

		sqlite3_result_int( ctx, value1 < value2 );
	}

	void SQLITE_UINT_SMALL_EQUAL_THAN( sqlite3_context* ctx,int argc, sqlite3_value** argv )
	{
		uint64_t value1 = *(uint64_t*)(sqlite3_value_blob(argv[0]));
		uint64_t value2 = *(uint64_t*)(sqlite3_value_blob(argv[1]));
//		LOG_PRINT_L0("blob value1  " << value1);
//		LOG_PRINT_L0("blob value2  " << value2);

		sqlite3_result_int( ctx, value1 <= value2 );
	}

	void SQLITE_UINT_EQUAL( sqlite3_context* ctx,int argc, sqlite3_value** argv )
	{
		uint64_t value1 = *(uint64_t*)(sqlite3_value_blob(argv[0]));
		uint64_t value2 = *(uint64_t*)(sqlite3_value_blob(argv[1]));
//		LOG_PRINT_L0("blob value1  " << value1);
//		LOG_PRINT_L0("blob value2  " << value2);

		sqlite3_result_int( ctx, value1 == value2 );
	}

	std::vector<std::string> statistics_tables_sql =
	{
			R"(CREATE TABLE IF NOT EXISTS t_next_block_difficulty (blockheight integer,timespan integer,totalwork integer,difficulty integer,logtime TIMESTAMP default (datetime('now', 'localtime')));)",
			R"(CREATE TABLE IF NOT EXISTS t_block_create_time (blockheight integer PRIMARY KEY,block_hash varchar(64),block_nonce varchar(64),difficulty integer,create_template_time integer,notify_block_time integer);)"
	};

  BlockchainSQLITEDB::~BlockchainSQLITEDB()
  {

  }

  BlockchainSQLITEDB::BlockchainSQLITEDB() :
			m_statistics_open(false),
			m_sqlite3_db(nullptr),
			m_sqlite3_stmt(nullptr)
  {

  }

  int BlockchainSQLITEDB::open(const std::string& filename,const int mdb_flags)
  {
    int ret = sqlite3_open_v2(filename.c_str(),&m_sqlite3_db, mdb_flags,nullptr);
    if(ret != SQLITE_OK)
    {
			LOG_ERROR("sqlite3 open failed ");
			return SQLITE_ERROR;
    }
    //add custom functions
		sqlite3_create_function(m_sqlite3_db, "SQLITE_UINT_BIG_THAN", 2, SQLITE_ANY, nullptr, SQLITE_UINT_BIG_THAN, nullptr, nullptr);
		sqlite3_create_function(m_sqlite3_db, "SQLITE_UINT_SMALL_THAN", 2, SQLITE_ANY, nullptr, SQLITE_UINT_SMALL_THAN, nullptr, nullptr);
		sqlite3_create_function(m_sqlite3_db, "SQLITE_UINT_BIG_EQUAL_THAN", 2, SQLITE_ANY, nullptr, SQLITE_UINT_BIG_EQUAL_THAN, nullptr, nullptr);
		sqlite3_create_function(m_sqlite3_db, "SQLITE_UINT_SMALL_EQUAL_THAN", 2, SQLITE_ANY, nullptr, SQLITE_UINT_SMALL_EQUAL_THAN, nullptr, nullptr);
		sqlite3_create_function(m_sqlite3_db, "SQLITE_UINT_EQUAL", 2, SQLITE_ANY, nullptr, SQLITE_UINT_EQUAL, nullptr, nullptr);

    for(auto it = statistics_tables_sql.begin();it != statistics_tables_sql.end();++it)
		{
			ret= sqlite3_prepare_v2(m_sqlite3_db, it->c_str(), -1, &m_sqlite3_stmt, nullptr);
			if(ret != SQLITE_OK)
			{
				LOG_ERROR("create sqlite data base failed illeagal sql " << *it << sqlite3_errmsg(m_sqlite3_db));
				return SQLITE_ERROR;
			}

			//execute sql
			sqlite3_step(m_sqlite3_stmt);

			//finalize the for execute next sql
			sqlite3_finalize(m_sqlite3_stmt);
		}

    return SQLITE_OK;
  }

  int BlockchainSQLITEDB::close()
  {
		int ret = sqlite3_close_v2(m_sqlite3_db);
		if(ret != SQLITE_OK)
		{
			LOG_ERROR("sqlite3 close failed ");
			return SQLITE_ERROR;
		}

		LOG_PRINT_L1("sqlite3 close success ");
    return SQLITE_OK;
  }

  void BlockchainSQLITEDB::open_statistics() {
      this->m_statistics_open = true;
  }

	void BlockchainSQLITEDB::close_statistics() {
      this->m_statistics_open = false;
  }

  int BlockchainSQLITEDB::insert_next_difficulty(uint64_t blockheight,uint64_t timespan,uint64_t totalwork,uint64_t difficulty)
  {
			CHECK_AND_NO_ASSERT_MES_L(m_statistics_open,-1,0,"statistics closed");
			int ret;
			char* error_msg = nullptr;
			std::string insert_next_difficulty_sql = "INSERT INTO t_next_block_difficulty(blockheight,timespan,totalwork,difficulty)"
																								"VALUES(?1,?2,?3,?4);";
			/**begin transaction*/
			ret = sqlite3_exec(m_sqlite3_db,"BEGIN TRANSACTION;",nullptr,nullptr,&error_msg);
			if (ret != SQLITE_OK)
			{
				LOG_PRINT_L0("insert next block difficulty statistics begin transaction failed :" << error_msg);
				sqlite3_free(error_msg);
				sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
				return SQLITE_ERROR;
			}

			ret= sqlite3_prepare_v2(m_sqlite3_db, insert_next_difficulty_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
			if (ret == SQLITE_OK) {
				LOG_PRINT_L1("insert difficulty sql prepare successed");

				//bind parameters
				sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&blockheight,sizeof(blockheight),nullptr);
				sqlite3_bind_blob(m_sqlite3_stmt,2,(void*)&timespan,sizeof(timespan),nullptr);
				sqlite3_bind_blob(m_sqlite3_stmt,3,(void*)&totalwork,sizeof(totalwork),nullptr);
				sqlite3_bind_blob(m_sqlite3_stmt,4,(void*)&difficulty,sizeof(difficulty),nullptr);

				//execute the sql
				sqlite3_step(m_sqlite3_stmt);
				LOG_PRINT_L1("insert difficulty sql stepped error msg " << sqlite3_errmsg(m_sqlite3_db));
				sqlite3_finalize(m_sqlite3_stmt);
				sqlite3_exec(m_sqlite3_db,"COMMIT;",nullptr,nullptr,&error_msg);

				return SQLITE_OK;
			}
			else {
				LOG_ERROR("sqlite 3 insert block difficulty  failed : " << sqlite3_errmsg(m_sqlite3_db));
				sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
				return SQLITE_ERROR;
			}

      return ret;
  }

	int BlockchainSQLITEDB::query_next_difficulty(uint64_t from_height,uint64_t to_height,std::vector<st_nextdifficulty_statistics> & results)
	{
		CHECK_AND_NO_ASSERT_MES_L(m_statistics_open,-1,0,"statistics closed");
		int ret;
		char* error_msg = nullptr;

		std::string query_nextdifficulty_statistics_sql = "SELECT blockheight,timespan,totalwork,difficulty,logtime FROM t_next_block_difficulty"
																									 " WHERE SQLITE_UINT_BIG_EQUAL_THAN(blockheight,?1) AND SQLITE_UINT_SMALL_EQUAL_THAN(blockheight,?2)";

		LOG_PRINT_L1("query sql is " << query_nextdifficulty_statistics_sql);
		ret= sqlite3_prepare_v2(m_sqlite3_db, query_nextdifficulty_statistics_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
		if (ret == SQLITE_OK) {
			int rows = 0;
			LOG_PRINT_L1("query next difficulty statistics prepare sql ok ");

			//bind parameters
			sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&from_height,sizeof(from_height), nullptr);
			sqlite3_bind_blob(m_sqlite3_stmt,2,(void*)&to_height,sizeof(to_height), nullptr);

			while (sqlite3_step(m_sqlite3_stmt) == SQLITE_ROW) {

				uint64_t blockheight = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 0);
				uint64_t timespan = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt,1);
				uint64_t  totalwork = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt,2);
				uint64_t difficulty = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 3);
				const char* logtime = (const char*)sqlite3_column_text(m_sqlite3_stmt, 4);

				st_nextdifficulty_statistics ns;
				ns.blockheight = blockheight;
				ns.timespan = timespan;
				ns.totalwork = totalwork;
				ns.difficulty = difficulty;
				ns.logtime = logtime;


				LOG_PRINT_L1("blockheight = " << blockheight
																			<< ", timespan = "<< timespan
																			<< ", totalwork = "<< totalwork
																			<< ", difficulty = "<< difficulty
																			<< ", logtime = " << logtime;);
				results.push_back(ns);
				rows ++;
			}
			LOG_PRINT_L1("get " << rows << " data from t_next_block_difficulty");
			sqlite3_finalize(m_sqlite3_stmt);
		}
		else {
			LOG_ERROR("query difficulty sql error ret code " << sqlite3_errmsg(m_sqlite3_db));
			return SQLITE_ERROR;
		}

		return ret;
	}

int BlockchainSQLITEDB::query_next_difficulty_by_height(uint64_t height,std::vector<st_nextdifficulty_statistics> & results)
{
	CHECK_AND_NO_ASSERT_MES_L(m_statistics_open,-1,0,"statistics closed");
	int ret;
	char* error_msg = nullptr;

	std::string query_nextdifficulty_statistics_sql = "SELECT blockheight,timespan,totalwork,difficulty,logtime FROM t_next_block_difficulty"
																										" WHERE SQLITE_UINT_EQUAL(blockheight,?1)";

	LOG_PRINT_L1("query sql is " << query_nextdifficulty_statistics_sql);
	ret= sqlite3_prepare_v2(m_sqlite3_db, query_nextdifficulty_statistics_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
	if (ret == SQLITE_OK) {
		int rows = 0;
		LOG_PRINT_L1("query next difficulty statistics prepare sql ok height " << height);

		//bind parameters
		sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&height,sizeof(height), nullptr);

		while (sqlite3_step(m_sqlite3_stmt) == SQLITE_ROW) {

			uint64_t blockheight = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 0);
			uint64_t timespan = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt,1);
			uint64_t  totalwork = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt,2);
			uint64_t difficulty = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 3);
			const char* logtime = (const char*)sqlite3_column_text(m_sqlite3_stmt, 4);

			st_nextdifficulty_statistics ns;
			ns.blockheight = blockheight;
			ns.timespan = timespan;
			ns.totalwork = totalwork;
			ns.difficulty = difficulty;
			ns.logtime = logtime;


			LOG_PRINT_L1("blockheight = " << blockheight
																		<< ", timespan = "<< timespan
																		<< ", totalwork = "<< totalwork
																		<< ", difficulty = "<< difficulty
																		<< ", logtime = " << logtime;);
			results.push_back(ns);
			rows ++;
		}
		LOG_PRINT_L1("get " << rows << " data from t_next_block_difficulty");
		sqlite3_finalize(m_sqlite3_stmt);
	}
	else {
		LOG_ERROR("query difficulty sql error ret code " << sqlite3_errmsg(m_sqlite3_db));
		return SQLITE_ERROR;
	}

	return ret;
}

  int BlockchainSQLITEDB::insert_block_statistics(uint64_t blockheight, uint64_t difficulty,uint64_t create_template_time)
  {
			CHECK_AND_NO_ASSERT_MES_L(m_statistics_open,-1,0,"statistics closed");
			int ret;
			char* error_msg = nullptr;
			/**insert for block create statistics */
			std::string insert_blockcreate_sql = "INSERT INTO t_block_create_time (blockheight,difficulty,create_template_time) VALUES(?1,?2,?3);";

			/**begin transaction*/
			ret = sqlite3_exec(m_sqlite3_db,"BEGIN TRANSACTION;",nullptr,nullptr,&error_msg);
			if (ret != SQLITE_OK)
			{
				LOG_PRINT_L0("insert block create statistics begin transaction failed :" << error_msg);
				sqlite3_free(error_msg);
				sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
				return SQLITE_ERROR;
			}

			ret= sqlite3_prepare_v2(m_sqlite3_db, insert_blockcreate_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
			if (ret == SQLITE_OK) {
				std::cout << "sqlite 3 insert block create statistics sql prepare successed " << std::endl;
				sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&blockheight,sizeof(blockheight),nullptr);
				sqlite3_bind_blob(m_sqlite3_stmt,2,(void*)&difficulty,sizeof(difficulty),nullptr);
				sqlite3_bind_blob(m_sqlite3_stmt,3,(void*)&create_template_time,sizeof(create_template_time),nullptr);

				sqlite3_step(m_sqlite3_stmt);
				sqlite3_finalize(m_sqlite3_stmt);
				sqlite3_exec(m_sqlite3_db,"COMMIT;",nullptr,nullptr,&error_msg);

				return SQLITE_OK;
			}
			else {
				std::cout << "sqlite 3 insert block create statistic failed : " << sqlite3_errmsg(m_sqlite3_db) << std::endl;
				sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
				return SQLITE_ERROR;
			}

      return ret;
  }

	int BlockchainSQLITEDB::update_block_statistics(uint64_t blockheight,std::string block_hash,
																									std::string block_nonce, uint64_t notify_block_time)
	{
		CHECK_AND_NO_ASSERT_MES_L(m_statistics_open,-1,0,"statistics closed");

		int ret;
		char* error_msg = nullptr;
		std::string update_blockcreate_sql = "update t_block_create_time set "
																					"block_hash = \'" +  block_hash +"\',"
																					+"block_nonce= \'" +  block_nonce + "\',"
																					+"notify_block_time = ?1 "
																					+"where blockheight = ?2;";

		/**begin transaction*/
		ret = sqlite3_exec(m_sqlite3_db,"BEGIN TRANSACTION;",nullptr,nullptr,&error_msg);
		if (ret != SQLITE_OK)
		{
			LOG_PRINT_L1("update block create statistics begin transaction failed :" << error_msg);
			sqlite3_free(error_msg);
			sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
			return SQLITE_ERROR;
		}

		ret= sqlite3_prepare_v2(m_sqlite3_db, update_blockcreate_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
		if (ret == SQLITE_OK) {
			LOG_PRINT_L1("sqlite 3 update block create statistics successed ");

			sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&notify_block_time,sizeof(notify_block_time),nullptr);
			sqlite3_bind_blob(m_sqlite3_stmt,2,(void*)&blockheight,sizeof(blockheight),nullptr);

			sqlite3_step(m_sqlite3_stmt);
			sqlite3_finalize(m_sqlite3_stmt);
			sqlite3_exec(m_sqlite3_db,"COMMIT;",nullptr,nullptr,&error_msg);
		}
		else {
			LOG_ERROR("sqlite 3 update block create statistic failed : " << sqlite3_errmsg(m_sqlite3_db));
			sqlite3_exec(m_sqlite3_db,"END TRANSACTION;",nullptr,nullptr,nullptr);
			return SQLITE_ERROR;
		}

		return SQLITE_OK;
	}

	int BlockchainSQLITEDB::query_block_statistics(uint64_t from_height,uint64_t to_blockheight, std::vector<st_blockcreate_statistics> results)
	{

		/**query block create statistics */
		std::string query_blockcreate_statistics_sql = "SELECT blockheight,block_hash,block_nonce,difficulty,create_template_time,notify_block_time FROM t_block_create_time"
																								 "WHERE SQLITE_UINT_BIG_THAN(blockheight,?1) AND SQLITE_UINT_SMALL_THAN(blockheight,?1)";
		int ret= sqlite3_prepare_v2(m_sqlite3_db, query_blockcreate_statistics_sql.c_str(), -1, &m_sqlite3_stmt, nullptr);
		if (ret == SQLITE_OK) {
			LOG_PRINT_L1("query block create statistics sql ok ");

			//bind parameters
			sqlite3_bind_blob(m_sqlite3_stmt,1,(void*)&from_height,sizeof(from_height), nullptr);
			sqlite3_bind_blob(m_sqlite3_stmt,2,(void*)&to_blockheight,sizeof(to_blockheight), nullptr);

			while (sqlite3_step(m_sqlite3_stmt) == SQLITE_ROW) {

				uint64_t blockheight = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 0);
				const char* block_hash = (const char*)sqlite3_column_text(m_sqlite3_stmt,1);
				const char* block_nonce = (const char*)sqlite3_column_text(m_sqlite3_stmt,2);
				uint64_t difficulty = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 3);
				uint64_t create_template_time = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 4);
				uint64_t notify_block_time = *(uint64_t*)sqlite3_column_blob(m_sqlite3_stmt, 5);

				st_blockcreate_statistics bs;
				bs.blockheight = blockheight;
				bs.block_hash = block_hash;
				bs.block_nonce = block_nonce;
				bs.create_template_time = create_template_time;
				bs.notify_block_time = notify_block_time;
				bs.difficulty = (uint64_t)sqlite3_column_int(m_sqlite3_stmt, 3);

				LOG_PRINT_L1("blockheight = " << blockheight
									<< ", block_hash = "<< block_hash
									<< ", block_nonce = "<< block_nonce
									<< ", difficulty = "<< difficulty
									<< ", create_template_time = " << create_template_time
									<< ", notify_block_time = " << notify_block_time
									);
				results.push_back(bs);
			}
		}
		else {
			LOG_ERROR("query difficulty sql error ret code " << sqlite3_errmsg(m_sqlite3_db));
			return SQLITE_ERROR;
		}

		return 0;
	}


