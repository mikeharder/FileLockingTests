﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;

namespace FileLockingTestsManaged.Util
{
    internal static class IOUtil
    {
        public static IEnumerable<string> GetFiles(string path)
        {
            return Directory.GetFiles(path, "*", SearchOption.AllDirectories)
                .Select(p => Path.GetRelativePath(path, p));
        }

        public static IEnumerable<string> GetDirectories(string path)
        {
            return Directory.GetDirectories(path, "*", SearchOption.AllDirectories)
                .Select(p => Path.GetRelativePath(path, p));
        }

        public static string GetTempDir()
        {
            var temp = Path.GetTempFileName();
            File.Delete(temp);
            Directory.CreateDirectory(temp);
            return temp;
        }

        public static void DeleteDir(string path)
        {
            // If delete fails (e.g. due to a file in use), retry once every second up to 20 times.
            for (var i = 0; i < 20; i++)
            {
                try
                {
                    var dir = new DirectoryInfo(path) { Attributes = FileAttributes.Normal };
                    foreach (var info in dir.GetFileSystemInfos("*", SearchOption.AllDirectories))
                    {
                        info.Attributes = FileAttributes.Normal;
                    }
                    dir.Delete(recursive: true);
                    break;
                }
                catch (DirectoryNotFoundException)
                {
                    break;
                }
                catch (FileNotFoundException)
                {
                    break;
                }
                catch (Exception)
                {
                    if (i < 19)
                    {
                        Thread.Sleep(TimeSpan.FromSeconds(1));
                    }
                    else
                    {
                        throw;
                    }
                }
            }
        }

    }
}
