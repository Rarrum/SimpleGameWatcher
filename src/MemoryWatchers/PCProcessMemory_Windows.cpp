#ifdef _WIN32

#include "PCProcessMemory.h"

#include <algorithm>

#include <Windows.h>
#include <TlHelp32.h>

struct PCProcessMemoryData
{
    PCProcessMemoryData() = default;
    PCProcessMemoryData(const PCProcessMemoryData&) = delete;
    PCProcessMemoryData& operator=(const PCProcessMemoryData&) = delete;

    PCProcessMemoryData(PCProcessMemoryData &&from)
    {
        operator=(std::move(from));
    }

    PCProcessMemoryData& operator=(PCProcessMemoryData &&from)
    {
        Free();

        hProcess = from.hProcess;
        from.hProcess = nullptr;
    }

    ~PCProcessMemoryData()
    {
        Free();
    }

    void Free()
    {
        if (hProcess)
            CloseHandle(hProcess);
        hProcess = nullptr;
    };

    HANDLE hProcess = nullptr;
};

namespace
{
    std::string WindowsStringToLowerString(const std::string &winString)
    {
        std::string lowerString;
        lowerString.resize(winString.size());
        std::transform(winString.begin(), winString.end(), lowerString.begin(), [](wchar_t in) { return std::tolower((char)in); });
        return lowerString;
    }

    std::unique_ptr<PCProcessMemoryData> CreateForPid(DWORD pid)
    {
        std::unique_ptr<PCProcessMemoryData> process = std::make_unique<PCProcessMemoryData>();
        process->hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!process->hProcess)
            return {};

        return std::move(process);
    }
}

PCProcessMemory::PCProcessMemory(std::unique_ptr<PCProcessMemoryData> &&from)
{
    data = std::move(from);
}

std::unique_ptr<PCProcessMemory> PCProcessMemory::FindProcess(std::function<bool(const std::string &processNameLower)> matchProcessName, std::function<bool(const PCProcessMemory &process)> matchProcessMemory)
{
    HANDLE hSnapshotProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshotProcess == INVALID_HANDLE_VALUE)
        return {};

    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshotProcess, &pe))
        return {};

    do
    {
        // filter on process name
        std::string processNameLower = WindowsStringToLowerString(pe.szExeFile);
        if (!matchProcessName(processNameLower))
            continue;

        std::unique_ptr<PCProcessMemoryData> potentialProcessData = CreateForPid(pe.th32ProcessID);
        if (!potentialProcessData)
            continue;

        std::unique_ptr<PCProcessMemory> potentialProcess = std::unique_ptr<PCProcessMemory>(new PCProcessMemory(std::move(potentialProcessData)));

        if (!matchProcessMemory(*potentialProcess))
            continue;

        return std::move(potentialProcess);

    } while (Process32Next(hSnapshotProcess, &pe));

    return {};
}

PCProcessMemory::~PCProcessMemory()
{
}

bool PCProcessMemory::IsStillAlive() const
{
    if (!data)
        return false;

    DWORD exitCode = 0;
    if (GetExitCodeProcess(data->hProcess, &exitCode))
        return exitCode == STILL_ACTIVE;

    return false;
}

void PCProcessMemory::ScanMemory(std::function<bool(uint8_t *start, uint8_t *end, uint64_t startAddress)> scanChunk) const
{
    if (!IsStillAlive())
        return;

    std::vector<uint8_t> pageMemory;

    for (uint64_t scanAddress = 0;;)
    {
        MEMORY_BASIC_INFORMATION mbi = {0};
        uint64_t ret = VirtualQueryEx(data->hProcess, (void*)scanAddress, &mbi, sizeof(mbi));
        if (!ret)
            break;

        uint64_t pageStart = (uint64_t)mbi.BaseAddress;
        uint64_t pageEnd = pageStart + mbi.RegionSize;

        if (mbi.State == MEM_COMMIT && (mbi.Type == MEM_PRIVATE || mbi.Type == MEM_IMAGE) && (mbi.Protect & PAGE_GUARD) != PAGE_GUARD)
        {
            //TODO: This would be faster if we could filter to just the heaps of the module in question - the windows APIs for heap traversing are super slow though, and just make this worse

            pageMemory.resize(mbi.RegionSize);

            SIZE_T readBytes = 0;
            if (!ReadProcessMemory(data->hProcess, (void*)pageStart, pageMemory.data(), mbi.RegionSize, &readBytes))
            {
                scanAddress = pageStart + mbi.RegionSize;
                continue;
            }

            bool shouldContinue = scanChunk(pageMemory.data(), pageMemory.data() + pageMemory.size(), (uint64_t)mbi.BaseAddress);
            if (!shouldContinue)
                break;
        }

        scanAddress = pageStart + mbi.RegionSize;
    }
}

void PCProcessMemory::ReadMemory(std::vector<uint8_t> &target, uint64_t address, uint32_t amount) const
{
    target.resize(amount);
    if (!ReadProcessMemory(data->hProcess, (void*)address, target.data(), amount, nullptr))
        target.clear();
}

#endif
