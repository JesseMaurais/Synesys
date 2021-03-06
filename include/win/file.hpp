#ifndef win_file_hpp
#define win_file_hpp "WIN32 File API"

#include "win.hpp"
#include "ptr.hpp"
#include "err.hpp"
#include "dir.hpp"
#include <cstring>
#include <string>

namespace sys::win
{
	struct file_info : BY_HANDLE_FILE_INFORMATION
	{
		file_info(HANDLE h)
		{
			if (not GetFileInformationByHandle(h, this))
			{
				sys::win::err(here);
			}
		}
	};

	struct find_data : fwd::unique, WIN32_FIND_DATA
	{
		HANDLE h;

		find_data(char const *path)
		{
			h = FindFirstFile(path, this);
			if (sys::win::fail(h))
			{
				sys::win::err(here, path);
			}
		}

		~find_data()
		{
			if (not sys::win::fail(h))
			{
				if (not FindClose(h))
				{
					sys::win::err(here);
				}
			}
		}

		bool next()
		{
			if (not FindNextFile(h, this))
			{
				if (GetLastError() != ERROR_NO_MORE_FILES)
				{
					sys::win::err(here);
				}
				return failure;
			}
			return success;
		}
	};
}

namespace sys
{
	class files : sys::win::find_data
	{
		class iterator
		{
			sys::win::find_data *that;
			bool flag;

		public:

			iterator(sys::win::find_data* ptr, bool end)
			: that(ptr), flag(end)
			{ }

			bool operator!=(iterator const &it) const
			{
				return it.that != that or it.flag != flag;
			}

			auto operator*() const
			{
				auto const c = std::strrchr(that->cFileName, '\\');
				return nullptr == c ? that->cFileName : c + 1;
			}

			auto operator->() const
			{
				return (LPWIN32_FIND_DATA) that;
			}

			auto& operator++()
			{
				flag = that->next();
				return *this;
			}
		};

		static auto sub(std::string path)
		{
			return path.append(path.ends_with(sys::sep::dir) ? "*" : "\\*");
		}

	public:

		files(char const *path) : find_data(data(sub(path)))
		{ }

		auto begin()
		{
			return iterator(this, sys::win::fail(h));
		}

		auto end()
		{
			return iterator(this, true);
		}
	};
}

#endif // file
