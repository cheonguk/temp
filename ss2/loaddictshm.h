#ifndef _LOADDICTSHM_H
#define _LOADDICTSHM_H


/*****************************************************************************
*******
此处修改
********
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
<<<<<<< HEAD
                          char* sval,
                          char* sprompttt);
// add commont
=======
                          char* sprompt);

>>>>>>> origin/master
#endif
