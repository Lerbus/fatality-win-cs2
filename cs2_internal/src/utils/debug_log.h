#pragma once

#include <cstdint>

#ifdef _DEBUG

#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string>

namespace utils::debug
{
	enum class level : WORD
	{
		trace = FOREGROUND_INTENSITY,
		info = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		ok = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		warning = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		error = FOREGROUND_RED | FOREGROUND_INTENSITY
	};

	inline std::mutex &log_mutex()
	{
		static std::mutex value;
		return value;
	}

	inline std::string &log_path()
	{
		static std::string value;
		return value;
	}

	inline void initialize_log()
	{
		std::lock_guard lock(log_mutex());
		if (!log_path().empty())
			return;

		// always log beside the game executable (cs2.exe)
		{
			char module_path[MAX_PATH]{};
			const auto length = GetModuleFileNameA(nullptr, module_path, MAX_PATH);
			DWORD end = (length && length < MAX_PATH) ? length : 0;
			while (end > 0 && module_path[end - 1] != '\\' && module_path[end - 1] != '/')
				--end;
			module_path[end] = '\0';
			log_path() = std::string(module_path) + "fatality_debug.log";
		}
		auto write_session_header = []()
		{
			const auto file = CreateFileA(log_path().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file == INVALID_HANDLE_VALUE)
				return;

			SYSTEMTIME time{};
			GetLocalTime(&time);
			char header[256]{};
			sprintf_s(header, "\n===== cs2_internal debug session %u-%u-%u %u:%u:%u =====\n",
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
			DWORD written{};
			WriteFile(file, header, static_cast<DWORD>(strlen(header)), &written, nullptr);
			CloseHandle(file);
		};

		write_session_header();
	}

	inline void log(level severity, const char *tag, const char *format, ...)
	{
		std::lock_guard lock(log_mutex());
		char message[4096]{};
		va_list args;
		va_start(args, format);
		_vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
		va_end(args);

		SYSTEMTIME time{};
		GetLocalTime(&time);
		const auto line = [&]()
		{
			char value[4600]{};
			sprintf_s(value, "[%02u:%02u:%02u.%03u][cs2_internal][%s] %s\n",
				time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, tag, message);
			return std::string(value);
		}();

		const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO previous{};
		const bool can_color = console != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(console, &previous);
		if (can_color)
			SetConsoleTextAttribute(console, static_cast<WORD>(severity));

		std::printf("%s", line.c_str() + 14);
		std::fflush(stdout);

		const auto file = CreateFileA(log_path().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
		{
			DWORD written{};
			WriteFile(file, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
			CloseHandle(file);
		}

		if (can_color)
			SetConsoleTextAttribute(console, previous.wAttributes);
	}

	inline void pointer(const char *name, const void *value)
	{
		log(value ? level::ok : level::error, value ? "PTR+" : "PTR!", "%s = %p", name, value);
	}

	inline void offset(const char *name, std::uintptr_t value)
	{
		log(level::trace, "OFFSET", "%s = 0x%llX", name, static_cast<unsigned long long>(value));
	}

	inline int seh_filter(const char *operation, EXCEPTION_POINTERS *info)
	{
		const auto code = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0u;
		const auto address = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionAddress : nullptr;
		log(level::error, "SEH", "%s failed: exception=0x%08lX address=%p", operation, code, address);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

#else

// Release builds log to file as well - without this a plain "nothing happens"
// on inject leaves no trace of how far initialization actually got.
#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

namespace utils::debug
{
	enum class level { trace, info, ok, warning, error };

	inline std::mutex &log_mutex()
	{
		static std::mutex value;
		return value;
	}

	inline std::string &log_path()
	{
		static std::string value;
		return value;
	}

	inline void initialize_log()
	{
		std::lock_guard lock(log_mutex());
		if (!log_path().empty())
			return;

		constexpr char configured_path[] =
			"E:\\SteamLibrary\\steamapps\\common\\Counter-Strike Global Offensive\\game\\csgo\\bin\\win64\\fatality_debug.log";

		const auto writable = [](const char *path) -> bool
		{
			const auto file = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file == INVALID_HANDLE_VALUE)
				return false;
			CloseHandle(file);
			return true;
		};

		std::string chosen{};
		(void)configured_path;
		{
			// always log beside the game executable (cs2.exe)
			char module_path[MAX_PATH]{};
			const auto length = GetModuleFileNameA(nullptr, module_path, MAX_PATH);
			DWORD end = (length && length < MAX_PATH) ? length : 0;
			while (end > 0 && module_path[end - 1] != '\\' && module_path[end - 1] != '/')
				--end;

			constexpr char filename[] = "fatality_debug.log";
			if (end && end + sizeof(filename) <= MAX_PATH)
			{
				for (DWORD i = 0; filename[i] != '\0'; ++i)
					module_path[end + i] = filename[i];
				module_path[end + sizeof(filename) - 1] = '\0';
				if (writable(module_path))
					chosen = module_path;
			}
		}

		if (chosen.empty())
			return;

		log_path() = chosen;

		const auto file = CreateFileA(log_path().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return;

		SYSTEMTIME time{};
		GetLocalTime(&time);
		char header[256]{};
		sprintf_s(header, "\n===== cs2_internal release session %u-%u-%u %u:%u:%u =====\n",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
		DWORD written{};
		WriteFile(file, header, static_cast<DWORD>(strlen(header)), &written, nullptr);
		CloseHandle(file);
	}

	inline void log(level, const char *tag, const char *format, ...)
	{
		if (log_path().empty())
			initialize_log();

		std::lock_guard lock(log_mutex());
		if (log_path().empty())
			return;

		char message[4096]{};
		va_list args;
		va_start(args, format);
		_vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
		va_end(args);

		SYSTEMTIME time{};
		GetLocalTime(&time);
		char line[4600]{};
		sprintf_s(line, "[%02u:%02u:%02u.%03u][cs2_internal][%s] %s\n",
			time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, tag, message);

		const auto file = CreateFileA(log_path().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
		{
			DWORD written{};
			WriteFile(file, line, static_cast<DWORD>(strlen(line)), &written, nullptr);
			CloseHandle(file);
		}
	}

	inline void pointer(const char *name, const void *value)
	{
		log(value ? level::ok : level::error, value ? "PTR+" : "PTR!", "%s = %p", name, value);
	}

	inline void offset(const char *name, std::uintptr_t value)
	{
		log(level::trace, "OFFSET", "%s = 0x%llX", name, static_cast<unsigned long long>(value));
	}

	inline int seh_filter(const char *operation, void *)
	{
		log(level::error, "SEH", "%s failed", operation);
		return 1;
	}
}

#endif
