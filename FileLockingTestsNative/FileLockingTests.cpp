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
        //public void ReadWriteConcurrent()
        //{
        //    var name = nameof(ReadWriteConcurrent);
        //    var path = CreateFile(name, new string('a', 10));

        //    var random = new Random();
        //    var duration = TimeSpan.FromSeconds(5);
        //    var sw = Stopwatch.StartNew();

        //    var writeTask = Task.Run(() = >
        //    {
        //        while (sw.Elapsed < duration)
        //        {
        //            // Ensure length is a multiple of 10 (verified during read)
        //            var length = random.Next(1, 1000) * 10;

        //            // var content = (char)('a' + random.Next(0, 25));
        //            var content = 'a';

        //            File.WriteAllText(path, new string(content, length));
        //        }
        //    });

        //    while (sw.Elapsed < duration)
        //    {
        //        using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
        //        {
        //            var length = (int)fs.Length;
        //            var buffer = new byte[length];
        //            var bytesRead = fs.Read(buffer, 0, length);

        //            var message = $"length: {length}, bytesRead: {bytesRead}, fs.Length: {fs.Length}, " +
        //                $"new FileInfo(path).Length: {new FileInfo(path).Length}";

        //            // bytesRead may be one of the following values:
        //            // * 0, because the file sometimes appears empty while being overwritten by the write thread
        //            // * 4096, because File.WriteAllText() splits the write into multiple WriteFile calls of size 4096
        //            // * A multiple of 10, because the length written is always a multiple of 10
        //            Assert.IsTrue(bytesRead == 0 || bytesRead == 4096 || bytesRead % 10 == 0, message);

        //            if (bytesRead > 0)
        //            {
        //                // Ensure all chars in string are the same (meaning we didn't read part of two different writes)
        //                var stringRead = Encoding.UTF8.GetString(buffer, 0, bytesRead);
        //                Assert.AreEqual(new string(stringRead[0], bytesRead), stringRead, message);
        //            }
        //        }
        //    }

        //    writeTask.Wait();
        //}


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