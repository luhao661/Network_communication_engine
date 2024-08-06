#pragma once

#include"CELL.hpp"

namespace engine {
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
				//用8KB存储65535个sockfd 8192byte*8 = 65536
				if (nSocketNum < 65535)
					nSocketNum = 65535;
				_nfdSize = nSocketNum / (8 * sizeof(char)) + 1;
#endif 
				_pfdset = (fd_set *)new char[_nfdSize];
				/*
				好处：
				在Windows下，fd_set可以存储固定数量的套接字，而Linux下fd_set的大小可以基于最大文件描述符的值来调整。
				通过动态分配内存，可以灵活地适应不同平台的需求，而不是依赖于平台特定的fd_set大小。

				直接在栈上分配一个大型的fd_set结构可能会导致栈溢出，
				特别是在栈空间较小的环境中。通过在堆上动态分配内存，可以避免这种风险，并允许分配更大的数据结构。
				*/
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
				else
				{
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

