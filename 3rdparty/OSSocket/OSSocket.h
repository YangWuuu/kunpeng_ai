#ifndef __OSSOCKET_H__
#define __OSSOCKET_H__

// �����linux ������OS_POSIX
//#define OS_WINDOWS

#ifdef WIN32
#define OS_WINDOWS
#pragma comment(lib,"ws2_32.lib")
#else
#define OS_POSIX
#endif // WIN32


#if defined (OS_WINDOWS)
/////////////////////////////////////////////////////////windows��
#include<winsock2.h>
#include <ws2ipdef.h>

typedef SOCKET                                  OS_SOCKET;
#define OS_INVALID_SOCKET                       INVALID_SOCKET

#elif defined (OS_POSIX)
///////////////////////////////////////////////////////////////// ����ϵͳ
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef int                                     OS_SOCKET;

#define OS_INVALID_SOCKET                       -1
#endif


/** @struct _STRUCT_OS_ADDR_T_
* 	@brief	��ַ�ṹ
*	@note
*/
typedef struct _STRUCT_OS_ADDR_T_
{
    union
    {
        struct sockaddr_in sin4;	/**< IPV4 ��ַ*/
    }SA;
}OS_ADDR_T;


/** @fn		OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto)
*  @brief	����Socket
*	@param	-I	 - int nAf                  Э��
*	@param	-I	 - int nType                ����
*	@param	-I	 - int nProto               Э��
*	@return OS_SOCKET                       ���
*/
OS_SOCKET OSOpenSocket(int nAf, int nType, int nProto);


/** @fn		void OSCloseSocket(OS_SOCKET hSOcket)
*  @brief	�ر�socket���
*	@param	-I	 - OS_SOCKET hSOcket        ���
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

/** @fn     int OSAccept(OS_SOCKET hSocket, OS_ADDR_T *pstAddr, unsigned int nTimeOut��? OS_SOCKET *pCntSocket)
*  @brief	����
*	@param  -I   - OS_SOCKET hSocket                                ���
*	@param  -I   - OS_ADDR_T * pstAddr                              ��ַ
*	@param  -I   - unsigned int nTimeOut��? OS_SOCKET * pCntSocket   ���Ӿ��
*	@return int                                                     0��ʾ�ɹ�������ʧ��
*/
int OSAccept(OS_SOCKET hSocket, OS_SOCKET *pCntSocket);

/** @fn		int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr)
*  @brief	������ַ
*	@param	-I	 - int nAf                  ��ַ��
*	@param	-I	 - const char * szAddr      ��ַ
*	@param	-I	 - unsigned short nPort     �˿�
*	@param	-I	 - OS_ADDR_T * pstAddr      ��ַ
*	@return int                             0��ʾ�ɹ�������ʧ��
*/
int OSMakeAddrByString(int nAf, const char *szAddr, unsigned short nPort, OS_ADDR_T *pstAddr);

/** @fn		int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen)
*  @brief	��������
*	@param	-I	 - OS_SOCKET hSocket        ���
*	@param	-I	 - char * pBuf              ����
*	@param	-I	 - unsigned int nLen        ����
*	@return int                             ���ط��͵ĳ���
*/
int OSSend(OS_SOCKET hSocket, char *pBuf, unsigned int nLen);

/** @fn		int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen)
*  @brief	��������
*	@param	-I	 - OS_SOCKET hSocket        ���
*	@param	-I	 - char * pBuf              ����
*	@param	-I	 - int nLen                 ����
*	@return int                             0��ʾ�ɹ�������ʧ��
*/
int OSRecv(OS_SOCKET hSocket, char *pBuf, int nLen);


/** @fn     int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket)
*  @brief	����socket
*	@param  -I   - char * szIP              ip
*	@param  -I   - unsigned int nPort       ���
*	@param  -O   - OS_SOCKET * pSocket      socket
*	@return int 0��ʾ�ɹ�������ʧ��
*/
int OSCreateSocket(char *szIP, unsigned int nPort, OS_SOCKET *pSocket);

/** @fn     int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket)
*  @brief	�����˿�
*	@param  -I   - unsigned int nPort       �˿�
*	@param  -I   - OS_SOCKET * pSocket      ���
*	@return int                             0��ʾ�ɹ�������ʧ��
*/
int OSListenSocket(unsigned int nPort, OS_SOCKET *pSocket);
#endif