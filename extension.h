#ifndef BRACKETS_EXTENSION_H
#define BRACKETS_EXTENSION_H

#include "xwalk_extension_public.h"
#include "picojson.h"

struct BracketsExtensionContext;

struct BracketsExtension : public CXWalkExtension {
private:
  static CXWalkExtensionContext* CreateContext(CXWalkExtension* self);
  static const char* GetJavascript(CXWalkExtension* self);
  static void Shutdown(CXWalkExtension* self);

  void InitializeHandlerMap();

  typedef void (*HandlerFunc)(const picojson::object& input,
                              picojson::object& output);
  HandlerFunc GetHandler(const std::string& command);

  typedef std::map<std::string, HandlerFunc> HandlerMap;
  HandlerMap handler_map_;

public:
  BracketsExtension();

  friend struct BracketsExtensionContext;
};

struct BracketsExtensionContext : public CXWalkExtensionContext {
private:
  BracketsExtension *extension_;

  static void HandleMessage(CXWalkExtensionContext* self, const char* message);
  static void Destroy(CXWalkExtensionContext* self);

public:
  BracketsExtensionContext(BracketsExtension *extension);
};

#endif  // BRACKETS_EXTENSION_H
