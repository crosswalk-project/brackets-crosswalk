// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BRACKETS_BRACKETS_PLATFORM_H_
#define CAMEO_SRC_BRACKETS_BRACKETS_PLATFORM_H_

#include <string>
#include <vector>

namespace platform {

// Need to be in sync with the brackets.fs.* error constants.
// FIXME(jeez): add a kDirectoryNotEmptyError representing errno ENOTEMPTY.
enum ErrorCode {
  kNoError = 0,
  kUnknownError = 1,
  kInvalidParametersError = 2,
  kNotFoundError = 3,
  kCannotReadError = 4,
  kUnsupportedEncodingError = 5,
  kCannotWriteError = 6,
  // kOutOfSpaceError = 7,
  // kNotFileError = 8,
  kNotDirectoryError = 9,
  // kFileExistsError = 10
};

ErrorCode GetFileModificationTime(const std::string& path, time_t& mtime, bool& is_dir);
ErrorCode ReadDir(const std::string& path, std::vector<std::string>& directoryContents);
ErrorCode ReadFile(const std::string& filename, const std::string& encoding, std::string& contents);
ErrorCode WriteFile(const std::string& path, const std::string& encoding, std::string& data);
ErrorCode OpenLiveBrowser(const std::string& url);
ErrorCode OpenURLInDefaultBrowser(const std::string& url);
ErrorCode Rename(const std::string& old_name, const std::string& new_name);
ErrorCode MakeDir(const std::string& path, int mode);
ErrorCode DeleteFileOrDirectory(const std::string& path);
ErrorCode MoveFileOrDirectoryToTrash(const std::string& path);
ErrorCode IsNetworkDrive(const std::string& path, bool& is_networkDrive);
ErrorCode ShowFolderInOSWindow(const std::string& path);
ErrorCode GetPendingFilesToOpen(std::vector<std::string>& directoryContents);
ErrorCode GetRemoteDebuggingPort(int& port);

}  // namespace platform

#endif  // CAMEO_SRC_BRACKETS_BRACKETS_PLATFORM_H_
