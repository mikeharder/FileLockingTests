#include "stdafx.h"
#include "CppUnitTest.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <thread>

#include <tchar.h>
#include <windows.h>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fs = std::filesystem;

namespace FileLockingTestsNative
{
    fs::path TempDir;

    TEST_MODULE_INITIALIZE(ModuleInit)
    {
        TempDir = tmpnam(nullptr);
        fs::create_directory(TempDir);
    }

    TEST_MODULE_CLEANUP(ModuleCleanup)
    {
        // fs::remove_all(TempDir);
    }

    TEST_CLASS(FileLockingTests)
    {
    public:
        TEST_METHOD(ReadWriteConcurrent)
        {
            fs::path path = MyCreateFile("ReadWriteConcurrent", string(10, 'a'));

            auto start = std::chrono::steady_clock::now();
            auto duration = std::chrono::seconds(5);

            std::thread writeThread([=]()
            {
                std::default_random_engine generator;
                std::uniform_int_distribution<int> distribution(1, 1000);

                while (std::chrono::steady_clock::now() - start < duration) {
                    auto length = distribution(generator) * 10;
                    auto str = string(length, 'a');

                    HANDLE handle = CreateFile(path.c_str(),
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        nullptr,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);

                    DWORD bytesWritten = 0;
                    WriteFile(handle, str.c_str(), length, &bytesWritten, NULL);

                    CloseHandle(handle);
                }
            });

            while (std::chrono::steady_clock::now() - start < duration) {
                LARGE_INTEGER li = {};
                HANDLE handle = CreateFile(path.c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr);

                GetFileSizeEx(handle, &li);

                std::string buffer(li.LowPart + 1, '\0');

                DWORD bytesRead = 0;
                ReadFile(handle, buffer.data(), li.LowPart, &bytesRead, NULL);

                Assert::IsTrue(bytesRead == 0 || bytesRead % 10 == 0, std::to_wstring(bytesRead).c_str());

                buffer.resize(bytesRead);

                if (bytesRead > 0) {
                    Assert::AreEqual(string(bytesRead, 'a'), buffer);
                }

                CloseHandle(handle);
            }

            writeThread.join();
        }

        fs::path MyCreateFile(std::string name, std::string content)
        {
            fs::path fullName = name;
            fullName += ".txt";

            fs::path path(TempDir / fullName);
            std::ofstream ostrm(path);
            ostrm << content;

            return path;
        }

        std::string MyReadFile(fs::path path)
        {
            LARGE_INTEGER li = {};
            HANDLE handle = CreateFile(path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

            GetFileSizeEx(handle, &li);

            std::string buffer(li.LowPart + 1, '\0');

            DWORD bytesRead = 0;
            ReadFile(handle, buffer.data(), li.LowPart, &bytesRead, NULL);
            buffer.resize(bytesRead);

            return buffer;
        }
    };
}
