
#include "loaddictfile.h"
#include "bb.h"

long GetDictShmId(long lShmkey);
long GetDictShmKey(void);
void FreeResult(DICTINFO* pResult);

static long CreateDictShmF(long lSize, DICTINFO* pResult, SHMINFO* pstShinfo);
long ReadDictFile(char* filename, DICTINFO** pResult, long* plCount, long* plSize);
long GetDictPromptFromFileShm(char* pDictShm, long  lDictCount, char* shs_key, char* sval, char* sprompt);

long LoadDictShm(void)
{    
    long        lRet; 
    long        lSize;
    long        lOffset;
    long        lCount;
    long        lShmId;
    long        lShmKey;
    char*       pShmMem;
    char        filename[512];
    DICTINFO*   pResult;
    SHMINFO         stShinfo;
    pResult     = NULL;
    
    lRet = INIscanf("PUBLIC", "DICTSHM_FILE", "%s", &filename);
    if (lRet < 0)
    {
        ErrLogF(__FILE__, "line = [%ld] ", __LINE__);
        printf("ExitToAgent(char *ps)");
        return -1;
    }
//strcpy(filename, "/ebfs/zhong/dict.dat");
    lRet = ReadDictFile(filename,
                        &pResult,
                        &lCount,
                        &lSize);
    if (lRet != 0)
    {
        printf("GetDictFileInfo err lRet=[%ld]\n", lRet);
        return -3;
    }
    
    stShinfo.lCount = lCount;   
    lRet = CreateDictShmF(lSize + sizeof(SHMINFO), pResult, &stShinfo);
    if (lRet != 0)
    {
        FreeResult(pResult);
        return -7;
    }

    FreeResult(pResult);
  /*  
    if (shmdt(pShmMem) == -1)
    {
        shmctl(lShmId, IPC_RMID, 0);
        printf("Shmdt error !\n");
        return -8;
    }
  */  
    return 0;    
}


static long CreateDictShmF(long lSize, DICTINFO* pResult, SHMINFO* pstShinfo)
{
    char* pShmMem;
    TBDICT stDict;
    DICTINFO* p;
    DICTINFO* q;
    long lOffset;
    long lShmid;
    long lShmKey;
     
    lShmKey = GetDictShmKey();
    if (lShmKey < 0)
    {
        printf("ExitToAgent(char *ps)\n");
        return -1;
    }
    printf("lShmKey=[%ld]\n", lShmKey);

    lShmid = shmget(lShmKey, 0, 0);

    if (lShmid != -1)
    {
        printf("ExitToAgent(char *ps)\n");
        return -1;
    }
    

    lShmid = shmget(lShmKey, lSize, 0666|IPC_CREAT);
    if (lShmid == -1)
    {
        printf("shmget err lShmKey=[%ld]\n", lShmKey);
        return -1;
    }

    pShmMem = shmat(lShmid, NULL, 0);    
    if (pShmMem == (char*)(-1))
    {
        shmctl(lShmid, IPC_RMID, 0);
        printf("shmat err\n");
        return -1;
    }

    memcpy(pShmMem, pstShinfo, sizeof(SHMINFO));
    p = pResult;    
    lOffset = 0;

    while(p->next != NULL)
    {
        memset(&stDict, 0, sizeof(TBDICT));
        strcpy(stDict.hs_key, Alltrim(p->hs_key));
        strcpy(stDict.val, Alltrim(p->val));
        strcpy(stDict.prompt, Alltrim(p->prompt));
        memcpy(pShmMem + sizeof(SHMINFO) + lOffset, &stDict, sizeof(TBDICT));
        lOffset = lOffset + sizeof(TBDICT);
        p = p->next;
    }

    strcpy(stDict.hs_key, Alltrim(p->hs_key));
    strcpy(stDict.val, Alltrim(p->val));
    strcpy(stDict.prompt, Alltrim(p->prompt));
    memcpy(pShmMem + sizeof(SHMINFO) + lOffset, &stDict, sizeof(TBDICT));
    
    return 0;
}

/*****************************************************************************
*****************************************************************************/
long GetDictShmId(long lShmkey)
{
    long    lRet;
    long    lShmId;
    long    lShmKey;
    
    lShmKey = GetDictShmKey();
    if (lShmKey < 0)
    {
        ErrLogF(__FILE__,
                "line = [%ld] get shm key err",
                __LINE__);
        return -1;
    }

    lShmId = shmget(lShmKey, 0, 0);
    if (lShmId == -1)
    {
        ErrLogF(__FILE__,
                "line=[%ld] shmget err lShmKey=[%ld]",
                __LINE__,
                lShmKey);
        return -1;
    }

    return lShmId;
}


/*****************************************************************************
*****************************************************************************/
long GetDictShmKey(void)
{
    long    lRet;
    long    lShmkey;

    lRet = INIscanf("PUBLIC", "DICTSHM_KEY", "%ld", &lShmkey);
//    lShmkey = 1001;
    if (lRet != 0)
    {
        ErrLogF(__FILE__,
                "line=[%ld] INIscanf lShmKey err lRet=[%ld]",
                __LINE__,
                lRet);
        return -1;
    }

    return lShmkey;
}


void FreeResult(DICTINFO* pResult)
{
    DICTINFO* p;
    DICTINFO* q;
    p = pResult;
    while(p->next != NULL)
    {
        q = (p->next);
        free(p);
        p = q;
    }
    free(p);
}
/*****************************************************************************
*****************************************************************************/
long ReadDictFile(char* filename, DICTINFO** pResult, long* plCount, long* plSize)
{
    FILE    *fp;
    char sBuf[256];
    char *p;
    DICTINFO* pStTemp;
    DICTINFO* pd;
    int iCount;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        ErrLogF(__FILE__,"ExitToAgent(char *ps)[%s]_!", filename);
        return -1;
    }
    *plCount = 0;
    while(!feof(fp))
    {
        memset(sBuf, 0, sizeof(sBuf));
        if (fgets(sBuf, sizeof(sBuf)-1, fp) == NULL)
        {
            break;
        }
        if(strlen(alltrim(sBuf)) == 0)
        {
            break;
        }
        p = sBuf;
        iCount = 0;
        while(*p != '\0')
        {
            if(*p == '|')
            {
                *p = ' ';
                iCount++;
            }
            p++;
        }
        if(iCount != 2)
        {
            fclose(fp);
            free(pStTemp);
            ErrLogF(__FILE__, "ExitToAgent(char *ps)_[%s] %d", sBuf, iCount);
            return -1;
        }
        pStTemp = NULL;
        while (pStTemp == NULL)
        {
            pStTemp = (DICTINFO *)malloc(sizeof(DICTINFO));
            memset(pStTemp, 0, sizeof(DICTINFO));
        }        
        
        sscanf(sBuf,
               "%s %s %s",
               pStTemp->hs_key,
               pStTemp->val,
               pStTemp->prompt);  
        pStTemp->next = NULL;    

        if (*pResult == NULL)
        {
            *pResult = pStTemp;
            pd = *pResult;
        }
        else
        {
            pd->next = pStTemp;
            pd = pd->next;
        //    (*pResult)->next = pStTemp;
            
        }
        (*plCount)++;
    }
    fclose(fp);
    *plSize = (*plCount) * sizeof(DICTINFO);
    return 0;
}
/*****************************************************************************
*****************************************************************************/
long UnLoadDictShm(void)
{
    long    lRet;
    long    lShmid;
    long    lShmeKey;

    lShmeKey = GetDictShmKey();
    if (lShmeKey < 0)
    {
        return -1;
    }

    lShmid = GetDictShmId(lShmeKey);
    if (lShmid == -1)
    {
        printf("fss shm does not exist shmkey=[%ld]\n", lShmeKey);
        return -2;
    }

    lRet = shmctl(lShmid, IPC_RMID, 0);
    if (lRet != 0)
    {
        printf("shmctl err shmkey=[%ld] shmid=[%ld] \
lRet=[%ld] errno=[%ld]\n",
               lShmeKey,
               lShmid,
               lRet,
               errno);
        return -3;
    }

    return 0;
}
/*****************************************************************************
*****************************************************************************/
long GetDictFilePrompt(char *shs_key,
                       char *sval,
                       char *sprompt)
{
    long    lRet;
    long    lShmid;
    long    lShmKey;
    char*   pshmaddr;
    char*   pDictShm;
    SHMINFO pShinfo;
    TBDICT  SDict;
    lShmKey = GetDictShmKey();
    if (lShmKey < 0)
    {
        ErrLogF(__FILE__,
                "line=[%ld] GetDictShmKey err",
                __LINE__);
        return -1;
    }
    lShmid = GetDictShmId(lShmKey);
    if (lShmid < 0)
    {
        ErrLogF(__FILE__,
                "line=[%ld] GetDictShmId err lRet=[%ld]",
                __LINE__,
                lShmid);
        return -1;
    }

    pshmaddr = shmat(lShmid, NULL, 0);
    if (pshmaddr == (char*)(-1))
    {
        ErrLogF(__FILE__,
                "line=[%ld] shmat err lshmid=[%ld]",
                __LINE__,
                lShmid);
        return -2;
    }

    memcpy(&pShinfo, pshmaddr, sizeof(SHMINFO));
    pDictShm = pshmaddr + sizeof(SHMINFO);
    lRet = GetDictPromptFromFileShm(pDictShm,
                                    pShinfo.lCount,
                                    shs_key,
                                    sval,
                                    sprompt);
    if (lRet != 0)  
    {
        ErrLogF(__FILE__,
                "line=[%ld] GetDictPromptFromShm err lRet=[%ld]",
                __LINE__,
                lRet);
        return -3;
    }
/*
    if (shmdt(pshmaddr) == -1)
    {
        ErrLogF(__FILE__,
                "line=[%ld] shmdt err",
                __LINE__);
        return -5;
    }
*/
    return 0;
}

/*****************************************************************************
*****************************************************************************/
long GetDictPromptFromFileShm(char* pDictShm,
                              long  lDictCount,
                              char* shs_key,
                              char* sval,
                              char* sprompt)
{
    long    lBegin;
    long    lEnd;
    long    lMid;
    int     iCmp;
    long    lOffset;
    TBDICT  SDict;

    Alltrim(shs_key);
    Alltrim(sval);
    if ((strlen(shs_key) == 0) || (strlen(sval) == 0))
    {
        ErrLogF(__FILE__, 
                "%ld ExitToAgent(char *ps) shs_key = [%s] sval = [%s]", 
                __LINE__,
                shs_key,
                sval);
        
        strcpy(sprompt, "");        
        return 0;
    }
     
    lOffset = 0;
    lBegin  = 0;
    lEnd    = lDictCount;

    while(lBegin <= lEnd)
    {
        lMid = (lBegin + lEnd) / 2;
        lOffset = lMid * sizeof(TBDICT);
        memcpy(&SDict, pDictShm + lOffset, sizeof(TBDICT));
        iCmp = memcmp(shs_key, SDict.hs_key, strlen(shs_key));
        if(iCmp == 0)
        {
            iCmp = memcmp(sval, SDict.val, strlen(sval));
            if(iCmp == 0)
            {
                strcpy(sprompt, SDict.prompt);
                return 0;
            }
            else if(iCmp > 0)
            {
                lBegin = lMid + 1;
            }
            else
            {
                lEnd = lMid - 1;
            }
        }
        else if(iCmp > 0)
        {
            lBegin = lMid + 1;
        }
        else
        {
            lEnd = lMid - 1;
        }
    }

    sprintf(sprompt,
            "ExitToAgent(char *ps)!hs_key=[%s] val=[%s]",
            shs_key,
            sval);

    return 0;
}
