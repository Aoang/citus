/*-------------------------------------------------------------------------
 *
 * split_shards.c
 *
 * This file contains functions to split a shard according to a given
 * distribution column value.
 *
 * Copyright (c) Citus Data, Inc.
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "distributed/worker_protocol.h"
#include "utils/typcache.h"


/* declarations for dynamic loading */
PG_FUNCTION_INFO_V1(isolate_tenant_to_new_shard);
PG_FUNCTION_INFO_V1(worker_hash);


/*
 * isolate_tenant_to_new_shard isolates a tenant to its own shard by spliting
 * the current matching shard.
 */
Datum
isolate_tenant_to_new_shard(PG_FUNCTION_ARGS)
{
	ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					errmsg("isolate_tenant_to_new_shard() is only supported on "
						   "Citus Enterprise")));
}


/*
 * worker_hash returns the hashed value of the given value.
 */
Datum
worker_hash(PG_FUNCTION_ARGS)
{
	Datum valueDatum = PG_GETARG_DATUM(0);

	CheckCitusVersion(ERROR);

	/* figure out hash function from the data type */
	Oid valueDataType = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TypeCacheEntry *typeEntry = lookup_type_cache(valueDataType,
												  TYPECACHE_HASH_PROC_FINFO);

	if (typeEntry->hash_proc_finfo.fn_oid == InvalidOid)
	{
		ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("cannot find a hash function for the input type"),
						errhint("Cast input to a data type with a hash function.")));
	}

	FmgrInfo *hashFunction = palloc0(sizeof(FmgrInfo));
	fmgr_info_copy(hashFunction, &(typeEntry->hash_proc_finfo), CurrentMemoryContext);

	/* calculate hash value */
	Datum hashedValueDatum =
		FunctionCall1Coll(hashFunction, PG_GET_COLLATION(), valueDatum);

	PG_RETURN_INT32(hashedValueDatum);
}
