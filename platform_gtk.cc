// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform.h"

#include <cerrno>
#include <cstdio>
#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace platform {

ErrorCode ConvertLinuxErrorCode(int errorCode, bool isReading=true)
{
  switch (errorCode) {
  case ENOENT:
    return kNotFoundError;
  case EACCES:
    return isReading ? kCannotReadError : kCannotWriteError;
  case ENOTDIR:
    return kNotDirectoryError;
  default:
    return kUnknownError;
  }
}

ErrorCode GetFileModificationTime(const std::string& path,
                                  time_t& mtime,
                                  bool& is_dir) {
  struct stat buf;
  if (stat(path.c_str(), &buf) == -1)
    // FIXME(jeez): handle kInvalidParametersError return case.
    return ConvertLinuxErrorCode(errno);

  mtime = buf.st_mtime;
  is_dir = S_ISDIR(buf.st_mode);
  return kNoError;
}

ErrorCode ReadDir(const std::string& path,
                  std::vector<std::string>& directoryContents) {
  GDir *dir = g_dir_open(path.c_str(), 0, NULL);

  if (!dir)
    return kUnknownError;

  std::vector<std::string> files;
  while (const char *name = g_dir_read_name(dir)) {
    if (g_file_test(name, G_FILE_TEST_IS_DIR))
      directoryContents.push_back(name);
    else
      files.push_back(name);
  }
  g_dir_close(dir);

  directoryContents.insert(directoryContents.end(), files.begin(), files.end());
  return kNoError;
}

ErrorCode ReadFile(const std::string& filename,
                   const std::string& encoding,
                   std::string& contents) {
  if (encoding != "utf8")
    return kUnsupportedEncodingError;

  char *file_contents;
  gsize length;
  if (!g_file_get_contents(filename.c_str(), &file_contents, &length, NULL))
    return kUnknownError;

  contents = std::string(file_contents, length);
  g_free(file_contents);

  return kNoError;
}

ErrorCode WriteFile(const std::string& path,
                    const std::string& encoding,
                    std::string& data) {
  if (encoding != "utf8")
    return kUnknownError;

  if (!g_file_set_contents(path.c_str(), data.c_str(), data.size(), NULL))
    return kUnknownError;

  return kNoError;
}

static bool TryOpenLiveBrowser(const std::string& browser_name,
                               const std::string& url,
                               GError **error) {
  std::string command_line = browser_name + \
          " --allow-file-access-from-files " + \
          url + " --remote-debugging-port=9222";
  return g_spawn_command_line_async(command_line.c_str(), error);
}

ErrorCode OpenLiveBrowser(const std::string& url) {
  GError *error;

  if (TryOpenLiveBrowser("google-chrome", url, &error))
    return kNoError;
  if (TryOpenLiveBrowser("chromium", url, &error))
    return kNoError;

  if (error) {
    std::cout << "Error when trying to launch live browser: " << error->message << "\n";
    g_error_free(error);
  }

  return kUnknownError;
}

ErrorCode OpenURLInDefaultBrowser(const std::string& url) {
  if (!gtk_show_uri(NULL, url.c_str(), GDK_CURRENT_TIME, NULL))
    return kUnknownError;
  return kNoError;
}

ErrorCode Rename(const std::string& old_name, const std::string& new_name) {
  if (rename(old_name.c_str(), new_name.c_str()) == -1)
    return kUnknownError;
  return kNoError;
}

ErrorCode MakeDir(const std::string& path, int mode) {
  // FIXME(jeez): remove this as soon as Brackets stop passing mode=0,
  // check brackets/src/file/NativeFileSystem.js, line 838, for more info.
  mode = mode | 0755;

  if (g_mkdir_with_parents(path.c_str(), mode) == -1)
    return ConvertLinuxErrorCode(errno);
  return kNoError;
}

ErrorCode DeleteFileOrDirectory(const std::string& path) {
  if (unlink(path.c_str()) == -1) {
    if (errno == EISDIR && (rmdir(path.c_str()) == 0)) // Then it is a directory.
      return kNoError;
    return ConvertLinuxErrorCode(errno);
  }
  return kNoError;
}

ErrorCode MoveFileOrDirectoryToTrash(const std::string& path) {
  GFile* file = g_file_new_for_path(path.c_str());
  gboolean success = g_file_trash(file, NULL, NULL);
  g_object_unref(file);
  return success ? kNoError : kUnknownError;
}

ErrorCode IsNetworkDrive(const std::string&, bool& is_networkDrive) {
  // FIXME(jeez): Add real implementation.
  is_networkDrive = false;
  return kNoError;
}

ErrorCode ShowFolderInOSWindow(const std::string& path) {
  std::string uri = "file://";
  uri.append(path);

  gtk_show_uri(NULL, uri.c_str(), GDK_CURRENT_TIME, NULL);

  return kNoError;
}

ErrorCode GetPendingFilesToOpen(std::vector<std::string>&) {
  // FIXME: Find a way through the command-line to obtain a list of files to
  // be opened by the editor. Maybe this should be implemented as a built-in
  // extension?
  return kNoError;
}

ErrorCode GetRemoteDebuggingPort(int& port) {
  // Ideally, this should be obtained from the command-line instead of returning
  // a hardcoded constant.
  port = 31000;
  return kNoError;
}

}  // namespace platform
