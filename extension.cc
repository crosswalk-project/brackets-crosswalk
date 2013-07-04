// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>
#include <map>
#include <string>

#include "brackets_api.h"
#include "extension.h"
#include "platform.h"

static picojson::value JSErrorFromPlatformError(platform::ErrorCode error_code) {
  // This works because platform.h is in sync with brackets.fs.* error
  // codes.
  return picojson::value(static_cast<double>(error_code));
}

BracketsExtension::HandlerFunc BracketsExtension::GetHandler(const std::string& command) {
  const HandlerMap::iterator iter = handler_map_.find(command);
  if (iter == handler_map_.end())
    return NULL;
  return iter->second;
}

void BracketsExtensionContext::HandleMessage(CCameoExtensionContext* self,
                                             const char* message) {
  picojson::value input;
  picojson::value output;
  
  picojson::parse(input, message, message + strlen(message));
  if (!input.is<picojson::object>())
    return;
  
  std::string cmd = input.get("cmd").get<std::string>();
  BracketsExtension::HandlerFunc handler =
        reinterpret_cast<BracketsExtensionContext*>(self)->extension_->GetHandler(cmd);
  if (!handler)
    return;
  
  picojson::object& input_map = input.get<picojson::object>();

  output = picojson::value(picojson::object());
  picojson::object& output_map = output.get<picojson::object>();
  std::string reply_id = input.get("_reply_id").get<std::string>();
  output_map["_reply_id"] = picojson::value(reply_id);

  handler(input_map, output_map);

  std::string result = output.serialize();
  cameo_extension_context_post_message(self, result.c_str());
}

void BracketsExtensionContext::Destroy(CCameoExtensionContext* self) {
  delete reinterpret_cast<BracketsExtensionContext*>(self);
}

BracketsExtensionContext::BracketsExtensionContext(BracketsExtension *extension)
      : extension_(extension) {
  destroy = &Destroy;
  handle_message = &HandleMessage;
}

void BracketsExtension::Shutdown(CCameoExtension* self) {
  delete reinterpret_cast<BracketsExtension*>(self);
}

const char* BracketsExtension::GetJavascript(CCameoExtension*) {
  return kGeneratedSource;
}

CCameoExtensionContext* BracketsExtension::CreateContext(CCameoExtension* self) {
  BracketsExtension* extension = reinterpret_cast<BracketsExtension*>(self);
  BracketsExtensionContext* context = new BracketsExtensionContext(extension);

  return reinterpret_cast<CCameoExtensionContext*>(context);
}

template <typename T> static T GetValueOrDefault(const picojson::object& object,
                                                 const std::string& key,
                                                 const T& default_value) {
  const picojson::object::const_iterator iter = object.find(key);
  if (iter == object.end())
    return default_value;
  if (!iter->second.is<T>())
    return default_value;
  return iter->second.get<T>();
}

static void HandleReadFile(const picojson::object& input,
                           picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  std::string encoding = GetValueOrDefault<std::string>(input, "encoding", "");
  std::string data;
  platform::ErrorCode error = platform::ReadFile(path, encoding, data);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
  if (error == platform::kNoError)
    output["data"] = picojson::value(data);
}

static void HandleGetFileModificationTime(const picojson::object& input,
                                          picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  time_t mtime;
  bool is_dir;
  platform::ErrorCode error =
        platform::GetFileModificationTime(path, mtime, is_dir);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
  if (error == platform::kNoError) {
    output["modtime"] = picojson::value((double)mtime);
    output["is_dir"] = picojson::value(is_dir);
  }
}

static void HandleReadDir(const picojson::object& input,
                          picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  std::vector<std::string> files_vector;
  platform::ErrorCode error = platform::ReadDir(path, files_vector);
  picojson::value files = picojson::value(picojson::array());

  if (error == platform::kNoError) {
    picojson::array& files_array = files.get<picojson::array>();
    for (size_t i = 0; i < files_vector.size(); i++)
      files_array.push_back(picojson::value(files_vector[i]));
  }

  output["files"] = files;
}

static void HandleWriteFile(const picojson::object& input,
                            picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  std::string data = GetValueOrDefault<std::string>(input, "data", "");
  std::string encoding = GetValueOrDefault<std::string>(input, "encoding", "");
  platform::ErrorCode error = platform::WriteFile(path, encoding, data);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleOpenLiveBrowser(const picojson::object& input,
                                  picojson::object& output) {
  std::string url = GetValueOrDefault<std::string>(input, "url", "");
  platform::ErrorCode error = platform::OpenLiveBrowser(url);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleOpenURLInDefaultBrowser(const picojson::object& input,
                                          picojson::object& output) {
  std::string url = GetValueOrDefault<std::string>(input, "url", "");
  platform::ErrorCode error = platform::OpenURLInDefaultBrowser(url);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleRename(const picojson::object& input,
                         picojson::object& output) {
  std::string old_path = GetValueOrDefault<std::string>(input, "old_path", "");
  std::string new_path = GetValueOrDefault<std::string>(input, "new_path", "");
  platform::ErrorCode error = platform::Rename(old_path, new_path);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleMakeDir(const picojson::object& input,
                          picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  int mode = static_cast<int>(input.find("mode")->second.get<double>());
  platform::ErrorCode error = platform::MakeDir(path, mode);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleDeleteFileOrDirectory(const picojson::object& input,
                                        picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  platform::ErrorCode error = platform::DeleteFileOrDirectory(path);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleMoveFileOrDirectoryToTrash(const picojson::object& input,
                                             picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  platform::ErrorCode error = platform::MoveFileOrDirectoryToTrash(path);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleIsNetworkDrive(const picojson::object& input,
                                 picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  bool is_network_drive;
  platform::ErrorCode error = platform::IsNetworkDrive(path, is_network_drive);

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
  output["is_network_drive"] = picojson::value(is_network_drive);
}

static void HandleShowOSFolder(const picojson::object& input,
                               picojson::object& output) {
  std::string path = GetValueOrDefault<std::string>(input, "path", "");
  platform::ErrorCode error = platform::ShowFolderInOSWindow(path);
  output["error"] = picojson::value(JSErrorFromPlatformError(error));
}

static void HandleGetPendingFilesToOpen(const picojson::object&,
                                        picojson::object& output) {
  std::vector<std::string> result; 
  platform::ErrorCode error = platform::GetPendingFilesToOpen(result);

  picojson::value files = picojson::value(picojson::array());
  picojson::array& files_array = files.get<picojson::array>();
  
  for (size_t i = 0; i < result.size(); i++)
    files_array.push_back(picojson::value(result[i]));

  output["error"] = picojson::value(JSErrorFromPlatformError(error));
  output["files"] = files;
}

static void HandleGetRemoteDebuggingPort(const picojson::object&,
                                         picojson::object& output) {
  int port;
  platform::ErrorCode error = platform::GetRemoteDebuggingPort(port);
  output["error"] = picojson::value(JSErrorFromPlatformError(error));
  output["debugging_port"] = picojson::value((double)port);
}

void BracketsExtension::InitializeHandlerMap() {
  handler_map_["ReadFile"] = HandleReadFile;
  handler_map_["GetFileModificationTime"] = HandleGetFileModificationTime;
  handler_map_["ReadDir"] = HandleReadDir;
  handler_map_["WriteFile"] = HandleWriteFile;
  handler_map_["OpenLiveBrowser"] = HandleOpenLiveBrowser;
  handler_map_["OpenURLInDefaultBrowser"] = HandleOpenURLInDefaultBrowser;
  handler_map_["Rename"] = HandleRename;
  handler_map_["MakeDir"] = HandleMakeDir;
  handler_map_["DeleteFileOrDirectory"] = HandleDeleteFileOrDirectory;
  handler_map_["MoveFileOrDirectoryToTrash"] = HandleMoveFileOrDirectoryToTrash;
  handler_map_["IsNetworkDrive"] = HandleIsNetworkDrive;
  handler_map_["ShowOSFolder"] = HandleShowOSFolder;
  handler_map_["GetPendingFilesToOpen"] = HandleGetPendingFilesToOpen;
  handler_map_["GetRemoteDebuggingPort"] = HandleGetRemoteDebuggingPort;
}

BracketsExtension::BracketsExtension() {
  name = "brackets";
  api_version = 1;
  get_javascript = &GetJavascript;
  shutdown = &Shutdown;
  context_create = &CreateContext;

  InitializeHandlerMap();
}

extern "C" CCameoExtension* cameo_extension_init(int32_t api_version) {
  if (api_version < 1)
    return NULL;
  return new BracketsExtension();
}
