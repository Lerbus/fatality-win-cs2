#include "game/game.h"
#include "utils/debug_log.h"

#include <Windows.h>

static void append_early_debug_log(const char *path, const char *message, bool reset = false)
{
	const auto file = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
		reset ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return;

	DWORD written{};
	DWORD length{};
	while (message[length] != '\0')
		++length;
	WriteFile(file, message, length, &written, nullptr);
	CloseHandle(file);
}

// the log always lives beside the game executable (cs2.exe)
static bool early_debug_log_path(char (&out)[MAX_PATH])
{
	const auto length = GetModuleFileNameA(nullptr, out, MAX_PATH);
	if (!length || length >= MAX_PATH)
		return false;

	DWORD end = length;
	while (end > 0 && out[end - 1] != '\\' && out[end - 1] != '/')
		--end;
	if (!end || end + 20 >= MAX_PATH)
		return false;

	const char filename[] = "fatality_debug.log";
	for (DWORD i = 0; filename[i] != '\0'; ++i)
		out[end + i] = filename[i];
	out[end + sizeof(filename) - 1] = '\0';
	return true;
}

static void write_early_debug_log(const char *message)
{
	char path[MAX_PATH]{};
	if (early_debug_log_path(path))
		append_early_debug_log(path, message);
}

static void reset_early_debug_log()
{
	char path[MAX_PATH]{};
	if (early_debug_log_path(path))
		append_early_debug_log(path, "", true);
}

#ifdef _DEBUG
static uintptr_t debug_instance{};
static uint32_t debug_reserved{};
static volatile LONG debug_initialization_started{};
static HANDLE debug_process_guard{};
static void init_debug_console();
static LONG WINAPI debug_unhandled_exception(EXCEPTION_POINTERS *info);

static DWORD WINAPI debug_initialization_thread(LPVOID)
{
	write_early_debug_log("[BOOT][EARLY] initialization worker started\r\n");
	init_debug_console();
	write_early_debug_log("[BOOT][EARLY] debug console setup returned\r\n");
	SetUnhandledExceptionFilter(debug_unhandled_exception);
	write_early_debug_log("[BOOT][EARLY] constructing game object\r\n");
	sdk::game = std::make_unique<sdk::game_t>(debug_instance, debug_reserved);
	write_early_debug_log("[BOOT][EARLY] game object constructed\r\n");
	sdk::game->init();
	write_early_debug_log("[BOOT][EARLY] game initialization returned\r\n");
	write_early_debug_log("[BOOT][EARLY] initialization worker exited\r\n");
	return 0;
}
#endif

#ifdef _DEBUG
#include <cstdio>

static void init_debug_console()
{
	write_early_debug_log("[BOOT][EARLY] initializing persistent logger\r\n");
	utils::debug::initialize_log();
	write_early_debug_log("[BOOT][EARLY] persistent logger returned\r\n");
	write_early_debug_log("[BOOT][EARLY] allocating debug console\r\n");
	const auto console_created = AllocConsole() != FALSE;
	write_early_debug_log(console_created ? "[BOOT][EARLY] debug console allocated\r\n" : "[BOOT][EARLY] debug console already exists\r\n");

	if (console_created)
	{
		write_early_debug_log("[BOOT][EARLY] redirecting debug streams\r\n");
		FILE *stream = nullptr;
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
		freopen_s(&stream, "CONIN$", "r", stdin);
		SetConsoleTitleA("cs2_internal debug");
	}

	write_early_debug_log("[BOOT][EARLY] writing debug logger banner\r\n");
	utils::debug::log(utils::debug::level::ok, "BOOT", "debug logging initialized; file=%s",
		utils::debug::log_path().c_str());
}

static LONG WINAPI debug_unhandled_exception(EXCEPTION_POINTERS *info)
{
	utils::debug::seh_filter("unhandled exception", info);
	utils::debug::log(utils::debug::level::error, "BOOT", "process will terminate after unhandled exception");
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

BOOL APIENTRY DllMain(uintptr_t inst, uint32_t reason, uint32_t reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
	#ifdef _DEBUG
		debug_process_guard = CreateMutexA(nullptr, FALSE, "Local\\fatality_cs2_initialization_guard");
		if (!debug_process_guard || GetLastError() == ERROR_ALREADY_EXISTS)
		{
			write_early_debug_log("[BOOT][EARLY] duplicate process initialization ignored\r\n");
			return TRUE;
		}

		if (InterlockedCompareExchange(&debug_initialization_started, 1, 0) != 0)
		{
			write_early_debug_log("[BOOT][EARLY] duplicate DLL attach ignored\r\n");
			return TRUE;
		}

		reset_early_debug_log();
		write_early_debug_log("[BOOT][EARLY] DllMain reached\r\n");
		debug_instance = inst;
		debug_reserved = reserved;
		const auto worker = CreateThread(nullptr, 0, debug_initialization_thread, nullptr, 0, nullptr);
		if (worker)
			CloseHandle(worker);
		else
			write_early_debug_log("[BOOT][EARLY] CreateThread failed\r\n");
	#else
		reset_early_debug_log();
		write_early_debug_log("[BOOT][EARLY] DllMain reached (release build)\r\n");
		std::thread(
			[inst, reserved]
			{
				write_early_debug_log("[BOOT][EARLY] release worker started\r\n");
				sdk::game = std::make_unique<sdk::game_t>(inst, reserved);
				write_early_debug_log("[BOOT][EARLY] game object constructed\r\n");
				sdk::game->init();
				write_early_debug_log("[BOOT][EARLY] game initialization returned\r\n");
			}).detach();
	#endif
	}

	return TRUE;
}
