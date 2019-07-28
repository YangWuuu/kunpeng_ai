#ifndef __OSSOCKET_H__
#define __OSSOCKET_H__

// 如果是linux ，定义OS_POSIX
//#define OS_WINDOWS

#ifdef WIN32
#define OS_WINDOWS
#pragma comment(lib,"ws2_32.lib")
#else
#define OS_POSIX
#endif // WIN32


#if defined (OS_WINDOWS)
/////////////////////////////////////////////////////////windows下
#include<winsock2.h>
#include <ws2ipdef.h>

typedef SOCKET                                  OS_SOCKET;
#define OS_INVALID_SOCKET                       INVALID_SOCKET

#elif defined (OS_POSIX)
///////////////////////////////////////////////////////////////// 其他系统
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef int                                     OS_SOCKET;

#define OS_INVALID_SOCKET                       -1
#endif


/** @struct _STRUCT_OS_ADDR_T_
* 	@brief	地址结构
*	@note
*/
typedef struct _STRUCT_OS_ADDR_T_
{
    union
    {
        struct sockaddr_in sin4;	/**< IPV4 地址*/
    }SA;
}OS_ADDR_T;


/** @fn		OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto)
*  @brief	创建Socket
*	@param	-I	 - int nAf                  协议
*	@param	-I	 - int nType                类型
*	@param	-I	 - int nProto               协议
*	@return OS_SOCKET                       句柄
*/
OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto);


/** @fn		void OSCloseSocket(OS_SOCKET hSOcket)
*  @brief	关闭socket句柄
*	@param	-I	 - OS_SOCKET hSOcket        句柄
*	@return void
*/
void OSCloseSocket(OS_SOCKET hSOcket);

/** @fn		int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
*  @brief
*	@param	-I	 - OS_SOCKET hSocket
*	@param	-I	 - OS_ADDR_T * pstAddr
*	@return int
*/
int OSConnect(OS_SOCKET hSocket, OS_ADDR_T *pstAddr);

/** @fn     int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr)
*  @brief
*	@param  -I   - OS_SOCKET hSocket
*	@param  -I   - OS_ADDR_T * pstAddr
*	@return int
*/
int OSBind(OS_SOCKET hSocket, OS_ADDR_T *pstAddr);

/** @fn     int OSAccept(OS_SOCKET hSocket, OS_ADDR_T *pstAddr, unsigned int nTimeOut￡? OS_SOCKET *pCntSocket)
*  @brief	接受
*	@param  -I   - OS_SOCKET hSocket                                句柄
*	@param  -I   - OS_ADDR_T * pstAddr                              地址
*	@param  -I   - unsigned int nTimeOut￡? OS_SOCKET * pCntSocket   链接句柄
*	@return int                                                     0表示成功，否则失败
*/
int OSAccept(OS_SOCKET hSocket, OS_SOCKET *pCntSocket);

/** @fn		int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr)
*  @brief	创建地址
*	@param	-I	 - int nAf                  地址族
*	@param	-I	 - const char * szAddr      地址
*	@param	-I	 - unsigned short nPort     端口
*	@param	-I	 - OS_ADDR_T * pstAddr      地址
*	@return int                             0表示成功，否则失败
*/
int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr);

/** @fn		int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen)
*  @brief	发送数据
*	@param	-I	 - OS_SOCKET hSocket        句柄
*	@param	-I	 - char * pBuf              缓存
*	@param	-I	 - unsigned int nLen        长度
*	@return int                             返回发送的长度
*/
int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen);

/** @fn		int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen)
*  @brief	接受数据
*	@param	-I	 - OS_SOCKET hSocket        句柄
*	@param	-I	 - char * pBuf              缓存
*	@param	-I	 - int nLen                 长度
*	@return int                             0表示成功，否则失败
*/
int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen);


/** @fn     int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket)
*  @brief	创建socket
*	@param  -I   - char * szIP              ip
*	@param  -I   - unsigned int nPort       句柄
*	@param  -O   - OS_SOCKET * pSocket      socket
*	@return int 0表示成功，否则失败
*/
int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket);

/** @fn     int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket)
*  @brief	监听端口
*	@param  -I   - unsigned int nPort       端口
*	@param  -I   - OS_SOCKET * pSocket      句柄
*	@return int                             0表示成功，否则失败
*/
int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket);
#endif