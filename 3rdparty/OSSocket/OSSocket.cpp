#include "OSSocket.h"
#include "stdlib.h"
#include "stdio.h"


#define  LOG_ERROR printf
#if defined (OS_WINDOWS)

/** @fn		OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto)
*  @brief	创建Socket
*	@param	-I	 - int nAf                  协议
*	@param	-I	 - int nType                类型
*	@param	-I	 - int nProto               协议
*	@return OS_SOCKET                       句柄
*/
OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto)
{
   return socket(nAf, nType, nProto);
}


/** @fn		void OSCloseSocket(OS_SOCKET hSOcket)
*  @brief	关闭socket句柄
*	@param	-I	 - OS_SOCKET hSOcket        句柄
*	@return void
*/
void OSCloseSocket(OS_SOCKET hSOcket)
{
   closesocket(hSOcket);
}



/** @fn		int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
*  @brief	建立奖励额
*	@param	-I	 - OS_SOCKET hSocket            句柄
*	@param	-I	 - OS_ADDR_T * pstAddr          地址
*	@return int                                 0表示成功，否则失败
*/
int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
{
   if (pstAddr == NULL)
   {
       return -1;
   }
   return connect(hSocket, (PSOCKADDR)&pstAddr->SA, sizeof(sockaddr_in));
}


/** @fn		int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr)
*  @brief	创建地址
*	@param	-I	 - int nAf                  地址族
*	@param	-I	 - const char * szAddr      地址
*	@param	-I	 - unsigned short nPort     端口
*	@param	-I	 - OS_ADDR_T * pstAddr      地址
*	@return int                             0表示成功，否则失败
*/
int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr)
{
   if (szAddr == NULL ||
       pstAddr == NULL)
   {
       return -1;
   }

   pstAddr->SA.sin4.sin_addr.s_addr = inet_addr(szAddr);
   if (pstAddr->SA.sin4.sin_addr.s_addr == INADDR_NONE)
   {
       return 0;
   }
   pstAddr->SA.sin4.sin_family = AF_INET;
   pstAddr->SA.sin4.sin_port = htons(nPort);

   return 0;
}


/** @fn		int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen)
*  @brief	发送数据
*	@param	-I	 - OS_SOCKET hSocket        句柄
*	@param	-I	 - char * pBuf              缓存
*	@param	-I	 - unsigned int nLen        长度
*	@return int                             返回发送的长度
*/
int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen)
{
   return send(hSocket, pBuf, nLen, 0);
}

/** @fn		int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen)
*  @brief	接受数据
*	@param	-I	 - OS_SOCKET hSocket        句柄
*	@param	-I	 - char * pBuf              缓存
*	@param	-I	 - int nLen                 长度
*	@return int                             0表示成功，否则失败
*/
int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen)
{
   return recv(hSocket, pBuf, nLen, 0);
}

/** @fn     int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
*  @brief
*	@param  -I   - OS_SOCKET hSocket
*	@param  -I   - OS_ADDR_T * pstAddr
*	@return int
*/
int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
{
   if (pstAddr == NULL)
   {
       return -1;
   }

   return bind(hSocket, (PSOCKADDR)&pstAddr->SA, sizeof(sockaddr_in));
}

#elif defined (OS_POSIX)
///////////////////////////////////////////////////////////////// 其他系统

/** @fn		OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto)
 *  @brief	创建Socket
 *	@param	-I	 - int nAf                  协议
 *	@param	-I	 - int nType                类型
 *	@param	-I	 - int nProto               协议
 *	@return OS_SOCKET                       句柄
 */
OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto) {
    return socket(nAf, nType, nProto);
}


/** @fn		void OSCloseSocket(OS_SOCKET hSOcket)
 *  @brief	关闭socket句柄
 *	@param	-I	 - OS_SOCKET hSOcket        句柄
 *	@return void
 */
void OSCloseSocket(OS_SOCKET hSOcket) {
    close(hSOcket);
}


/** @fn		int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
 *  @brief	建立奖励额
 *	@param	-I	 - OS_SOCKET hSocket            句柄
 *	@param	-I	 - OS_ADDR_T * pstAddr          地址
 *	@return int                                 0表示成功，否则失败
 */
int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr) {
    if (pstAddr == NULL) {
        return -1;
    }

    int nRet = 0;
    struct timeval timeout = {6, 0};           ///< 接受数据超时设置为3秒

    nRet = setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, sizeof(timeout));
    if (nRet != 0) {
        LOG_ERROR("setsockopt error no %d, strerror %s\n", errno, strerror(errno));
        return nRet;
    }

    nRet = connect(hSocket, (struct sockaddr *) &(pstAddr->SA), sizeof(sockaddr_in));
    if (nRet != 0) {
        LOG_ERROR("connect error no %d, strerror %s\n", errno, strerror(errno));
        return nRet;
    }
    return nRet;
}

/** @fn     int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
 *  @brief	
 *	@param  -I   - OS_SOCKET hSocket
 *	@param  -I   - OS_ADDR_T * pstAddr
 *	@return int
 */
int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr) {
    if (pstAddr == NULL) {
        return -1;
    }

    return bind(hSocket, (struct sockaddr *) &pstAddr->SA, sizeof(sockaddr_in));
}


/** @fn     int OSAccept(OS_SOCKET hSocket, OS_ADDR_T *pstAddr, unsigned int nTimeOut￡? OS_SOCKET *pCntSocket)
 *  @brief	接受
 *	@param  -I   - OS_SOCKET hSocket                                句柄
 *	@param  -I   - OS_ADDR_T * pstAddr                              地址
 *	@param  -I   - unsigned int nTimeOut￡? OS_SOCKET * pCntSocket   链接句柄  
 *	@return int                                                     0表示成功，否则失败
 */
int OSAccept(OS_SOCKET hSocket, OS_SOCKET *pCntSocket) {
    if (hSocket == OS_INVALID_SOCKET ||
        pCntSocket == NULL) {
        return -1;
    }

    *pCntSocket = accept(hSocket, NULL, NULL);

    if (*pCntSocket == OS_INVALID_SOCKET) {
        return -1;
    }

    return 0;
}


/** @fn		int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr)
 *  @brief	创建地址
 *	@param	-I	 - int nAf                  地址族
 *	@param	-I	 - const char * szAddr      地址
 *	@param	-I	 - unsigned short nPort     端口
 *	@param	-I	 - OS_ADDR_T * pstAddr      地址
 *	@return int                             0表示成功，否则失败
 */
int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr) {
    if (pstAddr == NULL) {
        return -1;
    }

    pstAddr->SA.sin4.sin_family = AF_INET;
    pstAddr->SA.sin4.sin_port = htons(nPort);

    if (szAddr == NULL) {
        pstAddr->SA.sin4.sin_addr.s_addr = htonl(INADDR_ANY);

    } else {
        pstAddr->SA.sin4.sin_addr.s_addr = inet_addr(szAddr);
    }


    return 0;
}


/** @fn		int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen)
 *  @brief	发送数据
 *	@param	-I	 - OS_SOCKET hSocket        句柄
 *	@param	-I	 - char * pBuf              缓存
 *	@param	-I	 - unsigned int nLen        长度
 *	@return int                             返回发送的长度
 */
int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen) {
    unsigned int nSendLen = 0;
    int nRet = 0;

    while (nSendLen < nLen) {
        struct timeval timeout = {6, 0};           ///< 接受数据超时设置为3秒

        nRet = setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, sizeof(timeout));
        if (nRet != 0) {
            LOG_ERROR("setsockopt error no %d, strerror %s\n", errno, strerror(errno));
            return nRet;
        }

        nRet = send(hSocket, pBuf + nSendLen, nLen - nSendLen, 0);
        if (nRet <= 0) {
            LOG_ERROR("send error no %d, strerror %s\n", errno, strerror(errno));
            return nRet;
        }

        nSendLen += nRet;
    }
    return nSendLen;
}

/** @fn		int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen)
 *  @brief	接受数据
 *	@param	-I	 - OS_SOCKET hSocket        句柄
 *	@param	-I	 - char * pBuf              缓存
 *	@param	-I	 - int nLen                 长度
 *	@return int                             0表示成功，否则失败
 */
int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen) {
    int nRet = 0;
    int nCount = 2;        /// 接受2次

    while (1) {
        struct timeval timeout = {6, 0};           ///< 接受数据超时设置为3秒

        // 减少应用计数
        nCount--;

        nRet = setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));
        if (nRet != 0) {
            // 失败直接报错
            LOG_ERROR("setsockopt error no %d, strerror %s\n", errno, strerror(errno));
            return nRet;
        }

        nRet = recv(hSocket, pBuf, nLen, 0);
        if (nRet <= 0) {
            // 失败了，重新接收
            LOG_ERROR("recv error no %d, strerror %s\n", errno, strerror(errno));
            continue;
        } else {
            break;
        }
    }
    return nRet;
}

#endif


/** @fn     int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket)
 *  @brief	创建socket
 *	@param  -I   - char * szIP              ip
 *	@param  -I   - unsigned int nPort       句柄
 *	@param  -O   - OS_SOCKET * pSocket      socket
 *	@return int 0表示成功，否则失败
 */
int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket) {
    if (pSocket == NULL ||
        szIP == NULL) {
        return -1;
    }

    *pSocket = OSOpenSocket(AF_INET, SOCK_STREAM, 0);
    if (*pSocket == OS_INVALID_SOCKET) {
        LOG_ERROR("Create socket error ip %s, port %d\n", szIP, nPort);
        return -1;
    }

    // 创建地址
    OS_ADDR_T stAddr;
    int nRet = 0;

    nRet = OSMakeAddrByString(AF_INET, szIP, nPort, &stAddr);
    if (nRet != 0) {
        OSCloseSocket(*pSocket);
        *pSocket = OS_INVALID_SOCKET;

        LOG_ERROR("OSMakeAddrByString error %d, ip %s, port %d\n", nRet, szIP, nPort);
        return -1;
    }

    // 连接
    nRet = OSConnect(*pSocket, &stAddr);
    if (nRet != 0) {
        OSCloseSocket(*pSocket);
        *pSocket = OS_INVALID_SOCKET;
        LOG_ERROR("OSConnect error %d ip %s, port %d\n", nRet, szIP, nPort);
        return -1;
    }

    return 0;
}


/** @fn     int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket)
 *  @brief	监听端口
 *	@param  -I   - unsigned int nPort       端口
 *	@param  -I   - OS_SOCKET * pSocket      句柄
 *	@return int                             0表示成功，否则失败
 */
int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket) {
    if (pSocket == NULL) {
        return -1;
    }

    *pSocket = OSOpenSocket(AF_INET, SOCK_STREAM, 0);
    if (*pSocket == OS_INVALID_SOCKET) {
        LOG_ERROR("Create socket error ip port %d\n", nPort);
        return -1;
    }

    // 创建地址
    OS_ADDR_T stAddr;
    int nRet = 0;

    nRet = OSMakeAddrByString(AF_INET, NULL, nPort, &stAddr);
    if (nRet != 0) {
        OSCloseSocket(*pSocket);
        *pSocket = OS_INVALID_SOCKET;

        LOG_ERROR("OSMakeAddrByString error %d,  port %d\n", nRet, nPort);
        return -1;
    }

    // 端口复用问题
    int nOpt = 1;
    setsockopt(*pSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &nOpt, sizeof(nOpt));

    // 连接
    nRet = OSBind(*pSocket, &stAddr);
    if (nRet != 0) {
        OSCloseSocket(*pSocket);
        *pSocket = OS_INVALID_SOCKET;
        LOG_ERROR("OSBind error %d,  port %d\n", nRet, nPort);
        return -1;
    }

    nRet = listen(*pSocket, 5);
    if (nRet != 0) {
        OSCloseSocket(*pSocket);
        *pSocket = OS_INVALID_SOCKET;
        LOG_ERROR("listen error %d, port %d\n", nRet, nPort);
        return -1;
    }

    return 0;
}
