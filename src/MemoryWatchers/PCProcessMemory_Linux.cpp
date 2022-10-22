#ifndef _WIN32

//NOTE:
// This does not work by default on linux, due to some security stuff.
// The user needs to first run this on the executable:
//    sudo setcap cap_sys_ptrace=eip EasyAutoTracker

#include "PCProcessMemory.h"

#include <filesystem>
#include <fstream>

#include <sys/uio.h>

namespace stdfs = std::filesystem;

namespace
{
    struct ProcessPage
    {
        uint64_t Start = 0;
        uint64_t End = 0;
    };

    struct ProcessInfo
    {
        uint64_t Id = 0;
        std::string Name;
        std::vector<ProcessPage> Pages;
    };

    std::vector<ProcessInfo> FindAllProcesses()
    {
        std::vector<ProcessInfo> allProcesses;

        // walk /proc/<id> and read from various files in there
        for (const auto& entry : stdfs::directory_iterator{"/proc/"})
        {
            if (!entry.is_directory())
                continue;

            ProcessInfo pi;
            pi.Id = std::strtoull(entry.path().filename().string().c_str(), nullptr, 10);
            if (pi.Id == 0 || pi.Id == std::numeric_limits<uint64_t>::max())
                continue;

            // Name comes from the status file
            std::ifstream statusFile(stdfs::path(entry.path() / "status"));
            for (std::string line; std::getline(statusFile, line); )
            {
                if (line.find("Name:") == 0)
                {
                    auto lineIter = line.begin() + 5;
                    while (lineIter != line.end() && std::isspace(*lineIter))
                        ++lineIter;

                    pi.Name = std::string(lineIter, line.end());
                    std::transform(pi.Name.begin(), pi.Name.end(), pi.Name.begin(), [](char in) { return std::tolower(in); });

                    break;
                }
            }

            if (pi.Name.empty())
                continue;

            // virtual address page ranges come from the maps file, of the form: 7ffe9f9ad000-7ffe9f9af000 <other stuff>
            std::ifstream mapsFile(stdfs::path(entry.path() / "maps"));
            for (std::string line; std::getline(mapsFile, line); )
            {
                auto iter = line.begin();
                while (iter != line.end() && *iter != '-')
                    ++iter;

                if (iter == line.end())
                    break;

                ProcessPage page;
                page.Start = std::strtoull(std::string(line.begin(), iter).c_str(), nullptr, 16);

                auto secondStart = ++iter;
                while (iter != line.end() && !std::isspace(*iter))
                    ++iter;

                page.End = std::strtoull(std::string(secondStart, iter).c_str(), nullptr, 16);

                if (page.Start == 0 || page.Start == std::numeric_limits<uint64_t>::max() || page.End == 0 || page.End == std::numeric_limits<uint64_t>::max())
                    continue;

                if (page.End == page.Start)
                    continue;

                pi.Pages.emplace_back(std::move(page));
            }

            if (pi.Pages.empty())
                continue;

            allProcesses.emplace_back(std::move(pi));
        }

        return allProcesses;
    }
}

struct PCProcessMemoryData
{
    ProcessInfo Info;
};

PCProcessMemory::PCProcessMemory(std::unique_ptr<PCProcessMemoryData> &&from)
{
    data = std::move(from);
}

std::unique_ptr<PCProcessMemory> PCProcessMemory::FindProcess(std::function<bool(const std::string &processNameLower)> matchProcessName, std::function<bool(const PCProcessMemory &process)> matchProcessMemory)
{
    std::vector<ProcessInfo> allProcesses = FindAllProcesses();

    for (ProcessInfo &pi : allProcesses)
    {
        if (!matchProcessName(pi.Name))
            continue;

        std::unique_ptr<PCProcessMemory> mem = std::unique_ptr<PCProcessMemory>(new PCProcessMemory(std::make_unique<PCProcessMemoryData>()));
        mem->data->Info = std::move(pi);

        if (matchProcessMemory(*mem))
            return mem;
    }

    return std::unique_ptr<PCProcessMemory>(new PCProcessMemory());
}

PCProcessMemory::~PCProcessMemory()
{
}

bool PCProcessMemory::IsStillAlive() const
{
    if (!data || data->Info.Id == 0)
        return false;

    return !ReadMemory(data->Info.Pages[data->Info.Pages.size() / 2].Start, 1).empty();
}

void PCProcessMemory::ScanMemory(std::function<bool(uint8_t *start, uint8_t *end, uint64_t startAddress)> scanChunk) const
{
    if (!data || data->Info.Id == 0)
        return;

    std::vector<uint8_t> pageMemory;

    for (const ProcessPage &page : data->Info.Pages)
    {
        uint64_t pageSize = page.End - page.Start;
        ReadMemory(pageMemory, page.Start, pageSize);
        if (pageMemory.empty())
            continue;

        if (!scanChunk(pageMemory.data(), pageMemory.data() + pageMemory.size(), page.Start))
            break;
    }
}

void PCProcessMemory::ReadMemory(std::vector<uint8_t> &target, uint64_t address, uint32_t amount) const
{
    if (!data || data->Info.Id == 0)
    {
        target.clear();
        return;
    }

    target.resize(amount);

    iovec localVec = {0};
    localVec.iov_base = target.data();
    localVec.iov_len = target.size();
    iovec remoteVec = {0};
    remoteVec.iov_base = (void*)address;
    remoteVec.iov_len = amount;
    ssize_t sizeRead = process_vm_readv(data->Info.Id, &localVec, 1, &remoteVec, 1, 0);
    if (sizeRead == -1)
        target.clear();
    else
        target.resize(sizeRead);
}

#endif
