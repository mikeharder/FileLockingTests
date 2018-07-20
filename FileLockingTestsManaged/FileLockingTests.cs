using NUnit.Framework;
using System;
using System.IO;
using System.Text;
using System.Threading;

namespace Tests
{
    public class FileLockingTests
    {
        [Test]
        public void FileShareRead_ReadSucceeds()
        {
            var name = nameof(FileShareRead_ReadSucceeds);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                Assert.AreEqual(name, File.ReadAllText(path));
            }
        }

        [Test]
        public void FileShareRead_WriteThrows()
        {
            var path = CreateFile(nameof(FileShareRead_WriteThrows));
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                try
                {
                    File.WriteAllText(path, "foo");
                    Assert.Fail("File.WriteAllText() did not throw");
                }
                catch (Exception e)
                {
                    Assert.IsAssignableFrom(typeof(IOException), e);
                }
            }
        }

        [Test]
        public void FileShareRead_DeleteThrows()
        {
            var path = CreateFile(nameof(FileShareRead_DeleteThrows));
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                try
                {
                    File.Delete(path);
                    Assert.Fail("File.Delete() did not throw");
                }
                catch (Exception e)
                {
                    Assert.IsAssignableFrom(typeof(IOException), e);
                }
            }
        }

        [Test]
        public void FileShareReadWriteDelete_ReadSucceeds()
        {
            var name = nameof(FileShareReadWriteDelete_ReadSucceeds);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                Assert.AreEqual(name, File.ReadAllText(path));
            }
        }

        [Test]
        public void FileShareReadWriteDelete_WriteSucceeds()
        {
            var name = nameof(FileShareReadWriteDelete_WriteSucceeds);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                File.WriteAllText(path, "foo");
                Assert.AreEqual("foo", File.ReadAllText(path));
            }
        }

        [Test]
        public void FileShareReadWriteDelete_DeleteSucceeds()
        {
            var name = nameof(FileShareReadWriteDelete_DeleteSucceeds);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                File.Delete(path);

                // File is not fully removed from disk since FileStream is still open
                Assert.True(File.Exists(path));
            }
            
            // File should be deleted after FileStream is closed
            Assert.False(File.Exists(path));
        }

        [Test]
        public void FileShareReadWriteDelete_ReadFileAfterWriteShorter()
        {
            var name = nameof(FileShareReadWriteDelete_ReadFileAfterWriteShorter);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                var lengthBeforeWrite = (int)fs.Length;

                var buffer = new byte[lengthBeforeWrite];
                var bytesRead = fs.Read(buffer, 0, lengthBeforeWrite);
                Assert.AreEqual(lengthBeforeWrite, bytesRead);
                Assert.AreEqual(name, Encoding.UTF8.GetString(buffer));

                fs.Seek(0, SeekOrigin.Begin);

                var shorterText = "a";
                File.WriteAllText(path, shorterText);

                buffer = new byte[lengthBeforeWrite];
                bytesRead = fs.Read(buffer, 0, lengthBeforeWrite);
                Assert.AreEqual(shorterText.Length, bytesRead);
                Assert.AreEqual(shorterText, Encoding.UTF8.GetString(buffer, 0, bytesRead));
            }
        }

        [Test]
        public void FileShareReadWriteDelete_ReadFileAfterWriteLonger()
        {
            var name = nameof(FileShareReadWriteDelete_ReadFileAfterWriteLonger);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                var lengthBeforeWrite = (int)fs.Length;

                var buffer = new byte[lengthBeforeWrite];
                var bytesRead = fs.Read(buffer, 0, lengthBeforeWrite);
                Assert.AreEqual(lengthBeforeWrite, bytesRead);
                Assert.AreEqual(name, Encoding.UTF8.GetString(buffer));

                fs.Seek(0, SeekOrigin.Begin);

                var longerText = new string('a', name.Length * 2);
                File.WriteAllText(path, longerText);

                buffer = new byte[lengthBeforeWrite];
                bytesRead = fs.Read(buffer, 0, lengthBeforeWrite);
                Assert.AreEqual(lengthBeforeWrite, bytesRead);
                Assert.AreEqual(new string('a', lengthBeforeWrite), Encoding.UTF8.GetString(buffer, 0, bytesRead));
            }
        }

        [Test]
        public void FileShareReadWriteDelete_ReadFileAfterDelete()
        {
            var name = nameof(FileShareReadWriteDelete_ReadFileAfterDelete);
            var path = CreateFile(name);
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete | FileShare.Write))
            {
                var lengthBeforeDelete = (int)fs.Length;

                var buffer = new byte[lengthBeforeDelete];
                var bytesRead = fs.Read(buffer, 0, lengthBeforeDelete);
                Assert.AreEqual(lengthBeforeDelete, bytesRead);
                Assert.AreEqual(name, Encoding.UTF8.GetString(buffer));

                fs.Seek(0, SeekOrigin.Begin);

                File.Delete(path);

                buffer = new byte[lengthBeforeDelete];
                bytesRead = fs.Read(buffer, 0, lengthBeforeDelete);
                Assert.AreEqual(lengthBeforeDelete, bytesRead);
                Assert.AreEqual(name, Encoding.UTF8.GetString(buffer));
            }
        }

        private static string CreateFile(string name)
        {
            var path = Path.Combine(AssemblySetUp.TempDir, $"{name}.txt");
            File.WriteAllText(path, name);
            return path;
        }
    }
}