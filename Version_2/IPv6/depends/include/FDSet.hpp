#ifndef _CELL_FDSET_HPP_
#define _CELL_FDSET_HPP_

#include"CELL.hpp"

//#define CELL_MAX_FD 10240
namespace doyou {
	namespace io {
		class FDSet
		{
		public:
			FDSet()
			{

			}

			~FDSet()
			{
				destory();
			}
			//Linux下表示socket fd的最大值
			//Windows下表示socket fd的数量
			void create(int MaxFds)
			{
				int nSocketNum = MaxFds;
#ifdef _WIN32
				if (nSocketNum < 64)
					nSocketNum = 64;
				_nfdSize = sizeof(u_int) + (sizeof(SOCKET)*nSocketNum);
#else
				//Linux下没有fd_count
				//用8KB存储65535个sockfd 8192byte*8 = 65536
				if (nSocketNum < 65535)
					nSocketNum = 65535;
				_nfdSize = nSocketNum / (8 * sizeof(char)) + 1;
#endif // _WIN32
				_pfdset = (fd_set *)new char[_nfdSize];
				memset(_pfdset, 0, _nfdSize);
				_MAX_SOCK_FD = nSocketNum;
			}

			void destory()
			{
				if (_pfdset)
				{
					delete[] _pfdset;
					_pfdset = nullptr;
				}
			}

			inline void add(SOCKET s)
			{
#ifdef _WIN32
				FD_SET(s, _pfdset);
#else
				if (s < _MAX_SOCK_FD)
				{
					FD_SET(s, _pfdset);
				}
				else {
					CELLLog_Error("FDSet::add sock<%d>, CELL_MAX_FD<%d>", (int)s, _MAX_SOCK_FD);
				}
#endif // _WIN32
			}

			inline void del(SOCKET s)
			{
				FD_CLR(s, _pfdset);
			}

			inline void zero()
			{
#ifdef _WIN32
				FD_ZERO(_pfdset);
#else
				memset(_pfdset, 0, _nfdSize);
#endif // _WIN32
			}

			inline bool has(SOCKET s)
			{
				return FD_ISSET(s, _pfdset);
			}

			inline fd_set* fdset()
			{
				return _pfdset;
			}

			void copy(FDSet& set)
			{
				memcpy(_pfdset, set.fdset(), set._nfdSize);
			}
		private:
			fd_set * _pfdset = nullptr;
			size_t _nfdSize = 0;
			int _MAX_SOCK_FD = 0;
		};
	}
}

/*
跨平台兼容性：
尽管代码根据操作系统（Windows和Linux）进行了条件编译，
这种设计使得同一套代码能够在不同的平台上运行，有助于提高代码的可移植性。
这在跨平台开发项目中尤为重要。
灵活管理大量连接：
在Linux环境下，默认处理为最大65535个socket描述符，
这意味着它能够应对高并发的socket连接管理需求。这对于开发高性能的网络应用
或服务器来说是一大优势。
动态内存管理：
代码【动态分配和释放内存来存储fd_set数据】，
这提高了内存使用的灵活性和效率。通过只分配必要的内存空间，减少了资源浪费。
错误和异常处理：
通过检查socket描述符是否超出预定的最大值，代码能够
及时发现并报告错误情况，比如尝试添加一个过大的文件描述符。
这有助于提前识别和防止潜在的问题，增强代码的健壮性。
增强的测试和调试能力：
封装后的FDSet可以提供更多的自检和状态检查接口，
有助于测试和调试。开发者可以更轻松地验证FDSet的状态，跟踪问题。
*/

#endif // !_CELL_FDSET_HPP_
