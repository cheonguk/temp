#ifndef _LOADDICTSHM_H
#define _LOADDICTSHM_H


/*****************************************************************************

*****************************************************************************/
long GetDictInfo(char*** pResult, long* plCount, long* plSize);

/*****************************************************************************
*****************************************************************************/
long CreateDictShm(long lCount, char** pResult, char* pShm);

/*****************************************************************************
*****************************************************************************/
long ReadCntSetBatch(char* pDictShm,
                          long  lDictCount,
                          char* shs_key,
                          char* sval,
                          char* sprompt);

#endif

